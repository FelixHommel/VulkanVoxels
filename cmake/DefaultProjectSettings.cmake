if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' because none other was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)

    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

find_program(CCACHE ccache)
if(CCACHE)
    message(STATUS "Using ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
else()
    message(STATUS "ccache not found")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(VVDEBUG "Use Debug statements and validation layers" ON)

if(VVDEBUG)
    add_compile_definitions(VVDEBUG)
elseif(VVDEBUG OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(VV_ENABLE_ASSERTS)
endif()
