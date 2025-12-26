find_package(Python3 REQUIRED COMPONENTS Interpreter)

option(VV_USE_SLANG "Use slang shaders over glsl" ON)
if(VV_USE_SLANG)
    # configure for slang shaders
    message(STATUS "using slang shaders")
    set(SHADER_SCRIPT ${CMAKE_SOURCE_DIR}/scripts/buildShadersSlang.py)
    set(SHADER_INPUT ${CMAKE_SOURCE_DIR}/resources/shaders/slang)
    set(SHADER_OUTPUT ${CMAKE_SOURCE_DIR}/resources/compiledShaders)
    set(SHADER_STAMP ${SHADER_OUTPUT}/build.stamp)
else()
    # configure for glsl shaders
    message(STATUS "using glsl shaders")
    set(SHADER_SCRIPT ${CMAKE_SOURCE_DIR}/scripts/buildShadersGLSL.py)
    set(SHADER_INPUT ${CMAKE_SOURCE_DIR}/resources/shaders)
    set(SHADER_OUTPUT ${CMAKE_SOURCE_DIR}/resources/compiledShaders)
    set(SHADER_STAMP ${SHADER_OUTPUT}/build.stamp)
endif()

# Compile the shaders
add_custom_command(
    OUTPUT ${SHADER_STAMP}
    COMMAND ${CMAKE_COMMAND} -E echo "Running python script to compile shaders..."
    COMMAND ${Python3_EXECUTABLE} ${SHADER_SCRIPT}
        --input ${SHADER_INPUT}
        --output ${SHADER_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E touch ${SHADER_STAMP}
    DEPENDS ${SHADER_SCRIPT}
    COMMENT "Compile shaders to SPIR-V"
    VERBATIM
)

# Make the command from above run automatically on cmake --build
add_custom_target(compile_shaders ALL
    DEPENDS ${SHADER_STAMP}
)
