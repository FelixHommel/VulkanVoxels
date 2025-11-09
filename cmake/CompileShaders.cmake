# Build glsl shaders automatically
# find_package(Python3 REQUIRED COMPONENTS Interpreter)
#
# set(SHADER_SCRIPT ${CMAKE_SOURCE_DIR}/scripts/buildShadersGLSL.py)
# set(SHADER_INPUT ${CMAKE_SOURCE_DIR}/src/shaders)
# set(SHADER_OUTPUT ${CMAKE_SOURCE_DIR}/shaders)
# set(SHADER_STAMP ${SHADER_OUTPUT}/build.stamp)
#
# # Command used to do the calling of the compile script
# add_custom_command(
#     OUTPUT ${SHADER_STAMP}
#     COMMAND ${CMAKE_COMMAND} -E echo "Running python script to compile shaders..."
#     COMMAND ${Python3_EXECUTABLE} ${SHADER_SCRIPT}
#         --input ${SHADER_INPUT}
#         --output ${SHADER_OUTPUT}
#     COMMAND ${CMAKE_COMMAND} -E touch ${SHADER_STAMP}
#     DEPENDS ${SHADER_SCRIPT}
#     COMMENT "Compile (vertex and fragment) shaders to SPIR-V"
#     VERBATIM
# )
#
# # Make the command from above run automatically on cmake --build
# add_custom_target(compile_shaders ALL
#     DEPENDS ${SHADER_STAMP}
# )
