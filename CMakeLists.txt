cmake_minimum_required(VERSION 3.24)

project(system_R
    VERSION 0.0.1
    DESCRIPTION "Open-source grandMA3-style lighting console"
    LANGUAGES C CXX)

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Put all binaries in one place so the app + tests are easy to find.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Default to a sensible build type for single-config generators.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

option(SYSTEMR_BUILD_TESTS "Build the unit-test suite" ON)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
include(FetchContent)

# JUCE 8 — the UI / app framework. Pinned; override the source dir for offline
# builds with -DFETCHCONTENT_SOURCE_DIR_JUCE=/path/to/local/JUCE.
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.9
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------------------
# Project targets
# ---------------------------------------------------------------------------
add_subdirectory(libs/core)
add_subdirectory(app)

if(SYSTEMR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
