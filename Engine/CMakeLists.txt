# Engine/CMakeLists.txt

# Shared library for the engine
add_library(Engine SHARED)

# ------------------------------------------------------------------
# Engine sources
# ------------------------------------------------------------------
file(GLOB_RECURSE ENGINE_SOURCES
    src/*.cpp
    src/*.h
)

target_sources(Engine PRIVATE ${ENGINE_SOURCES})

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
# Link Engine with GLAD + GLFW
# ------------------------------------------------------------------
target_link_libraries(Engine
    PUBLIC
        glad
        glfw
)

# Proper DLL export
target_compile_definitions(Engine PRIVATE ENGINE_EXPORTS)

set_target_properties(Engine PROPERTIES
    DEBUG_POSTFIX "d"                  # Engined.dll in Debug builds
    WINDOWS_EXPORT_ALL_SYMBOLS ON      # optional fallback
)