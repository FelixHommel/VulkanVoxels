cmake_minimum_required(VERSION 3.30)
project(
    RRenderer
    DESCRIPTION "A Renderer using Vulkan written in C++"
    LANGUAGES CXX
)

option(RRDEBUG "Use Debug and validation layers" ON)
if(RRDEBUG)
    add_definitions(-DRRDEBUG)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/DefaultProjectSettings.cmake)

add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

include(cmake/CompileShaders.cmake)

find_package(Vulkan REQUIRED)

add_subdirectory(deps)
add_subdirectory(src)
