cmake_minimum_required(VERSION 4.0.0)

project(job-scheduler
    VERSION 0.1.0
    DESCRIPTION "A C++ job scheduling library"
    LANGUAGES CXX
)

# Global settings
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Options
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer (Debug only)" ON)

# Compiler flags
add_library(compiler_options INTERFACE)

target_compile_options(compiler_options INTERFACE
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)

if(ENABLE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(compiler_options INTERFACE -fsanitize=address,undefined)
    target_link_options(compiler_options INTERFACE    -fsanitize=address,undefined)
endif()

# Library
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*main\\.cpp$")

add_library(scheduler-lib ${SOURCES})
target_link_libraries(scheduler-lib PUBLIC compiler_options)
target_include_directories(scheduler-lib
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Examples
if(BUILD_EXAMPLES)
    add_executable(scheduler-cli examples/main.cpp examples/CLI.cpp)
    target_link_libraries(scheduler-cli PRIVATE scheduler-lib)
    target_include_directories(scheduler-cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/examples)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
