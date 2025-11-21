# Engine/CMakeLists.txt

# Shared library for the engine
add_library(Engine SHARED)

# Make sure the target advertises C++20 so Windows/MSVC consumers match the engine ABI.
target_compile_features(Engine PUBLIC cxx_std_20)

# ------------------------------------------------------------------
# Engine sources
# ------------------------------------------------------------------
file(GLOB_RECURSE ENGINE_SOURCES
    src/*.cpp
    src/*.h
)

# Shader sources are tracked so IDEs show them and they are copied next to the binaries.
file(GLOB ENGINE_SHADER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine/Renderer/Shaders/*
)

target_sources(Engine PRIVATE ${ENGINE_SOURCES})
target_sources(Engine PRIVATE ${ENGINE_SHADER_SOURCES})

# ------------------------------------------------------------------
# Include directories
# ------------------------------------------------------------------
target_include_directories(Engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src          # so Game can #include <Engine/...>
        ${CMAKE_CURRENT_SOURCE_DIR}/include      # optional public headers
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include
)

# ------------------------------------------------------------------
# GLAD library (single C file + headers)
#   Expected at: Engine/vendor/glad/src/glad.c
# ------------------------------------------------------------------
add_library(glad STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/src/glad.c
)

target_include_directories(glad
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include
)

# On Windows, link against opengl32; on others, use standard OpenGL
if(WIN32)
    target_link_libraries(glad PUBLIC opengl32)
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(glad PUBLIC OpenGL::GL)
endif()

# ------------------------------------------------------------------
# GLFW (as subdirectory)
#   Expected at: Engine/vendor/glfw
#   This is the official GLFW CMake project.
# ------------------------------------------------------------------
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)

# ------------------------------------------------------------------
# Header-only libraries (glm, stb, EnTT)
#   Provide lightweight INTERFACE targets so consumers can just link them.
# ------------------------------------------------------------------
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb)

add_library(entt INTERFACE)
target_include_directories(entt INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt/single_include
)

# ------------------------------------------------------------------
# spdlog (as header-only)
#   Keep it lightweight by disabling installs/tests and enabling header-only mode.
# ------------------------------------------------------------------
set(SPDLOG_HEADER_ONLY ON CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog)

# ------------------------------------------------------------------
# yaml-cpp (static library)
#   Used for serialization and configuration handling.
# ------------------------------------------------------------------
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/yaml-cpp)

# ------------------------------------------------------------------
# Link Engine with GLAD + GLFW + vendor stack
# ------------------------------------------------------------------
target_link_libraries(Engine
    PUBLIC
        glad
        glfw
        glm
        stb
        entt
        spdlog::spdlog_header_only
        yaml-cpp
)

# Proper DLL export
target_compile_definitions(Engine PRIVATE ENGINE_BUILD_DLL)

set_target_properties(Engine PROPERTIES
    DEBUG_POSTFIX "d"                  # Engined.dll in Debug builds
    WINDOWS_EXPORT_ALL_SYMBOLS ON      # optional fallback
)

# Copy shader sources next to the built binaries for runtime loading.
add_custom_command(TARGET Engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine/Renderer/Shaders
        $<TARGET_FILE_DIR:Engine>/Shaders
)