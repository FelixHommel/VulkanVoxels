set(NAME "VulkanVoxelsApp")

include (${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_executable(${NAME}
    main.cpp
)

target_compile_features(${NAME} PRIVATE cxx_std_23)
target_link_libraries(${NAME}
    INTERFACE
        project_warnings
    PRIVATE
        VulkanVoxelsEngine
)
