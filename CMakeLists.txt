cmake_minimum_required(VERSION 3.30)
project(
    VulkanVoxels
    DESCRIPTION "A Renderer using Vulkan written in C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(PROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

include(cmake/DefaultProjectSettings.cmake)
include(cmake/CodeCoverage.cmake)

add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

include(cmake/CompileShaders.cmake)

find_package(Vulkan REQUIRED)

enable_testing()
add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(tests)
