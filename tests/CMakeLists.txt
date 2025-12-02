set(TEST_NAME "VulkanVoxelsTest")

include (GoogleTest)
include (${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_executable(${TEST_NAME}
    ./utility/CameraTest.cpp
    ./utility/ObjectTest.cpp
    ./utility/TransformTest.cpp
)

target_compile_features(${TEST_NAME} PRIVATE cxx_std_23)
target_link_libraries(${TEST_NAME}
    PRIVATE
        project_warnings
        VulkanVoxelsEngine
        spdlog::spdlog
        glm::glm
    PUBLIC
        GTest::gtest_main
)

gtest_discover_tests(${TEST_NAME})