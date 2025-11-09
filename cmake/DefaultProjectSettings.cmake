if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting buidl type to 'Debug' because none other was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)

    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

find_program(CCACHE ccache)
if(CCACHE)
    message(STATUS "Using ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
else()
    message(STATUS "Ccache not found")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DEBUG "Enable debug statements and asserts" OFF)
if(DEBUG OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG APP_ENABLE_ASSERTS)
endif()
