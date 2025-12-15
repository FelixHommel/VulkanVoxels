set(TEST_NAME "VulkanVoxelsTest")

include(GoogleTest)
include(${PROJECT_SOURCE_DIR}/cmake/StaticAnalyzers.cmake)

add_executable(${TEST_NAME}
    ./main.cpp
    ./core/BufferTest.cpp
    ./core/TextureTest.cpp
    ./mocks/MockInputHandler.cpp
    ./utility/CameraTest.cpp
    ./utility/KeyboardMovementControllerTest.cpp
    ./utility/ModelTest.cpp
    ./utility/ObjectTest.cpp
    ./utility/TransformTest.cpp
    ./utility/UtilsTest.cpp
    ./utility/VertexTest.cpp
    ./utility/exceptions/ExceptionTest.cpp
)

target_sources(${TEST_NAME}
    PRIVATE
        FILE_SET HEADERS
        FILES
            ./fixtures/TestVulkanContext.hpp
            ./helper/BufferTestHelper.hpp
            ./helper/RandomNumberGenerator.hpp
            ./mocks/MockInputHandler.hpp
)

target_compile_features(${TEST_NAME} PRIVATE cxx_std_23)
target_include_directories(${TEST_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TEST_NAME}
    PRIVATE
        glm::glm
        GTest::gtest
        gmock
        project_warnings
        spdlog::spdlog
        VulkanVoxelsEngine
)

gtest_discover_tests(${TEST_NAME})
