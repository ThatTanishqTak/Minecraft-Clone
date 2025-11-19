cmake_minimum_required(VERSION 3.27)
project(Minecraft-Clone LANGUAGES C CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------
# Nice unified output directories (Windows-Release-x64, etc.)
# ------------------------------------------------------------------
set(OUTPUT_CONFIG "$<CONFIG>")
if(NOT CMAKE_GENERATOR MATCHES "Visual Studio")
    set(OUTPUT_CONFIG ${CMAKE_BUILD_TYPE})
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH "x64")
else()
    set(ARCH "x86")
endif()

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}-${OUTPUT_CONFIG}-${ARCH}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${PROJECT_NAME})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${PROJECT_NAME})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${PROJECT_NAME})

# Parallel build on MSVC
if(MSVC)
    add_compile_options(/MP)
endif()

# ------------------------------------------------------------------
# Subdirectories
# ------------------------------------------------------------------
add_subdirectory(Engine)
add_subdirectory(Game)