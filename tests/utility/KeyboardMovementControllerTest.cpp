#include "utility/KeyboardMovementController.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "mocks/MockInputHandler.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "gtest/gtest.h"

#include <cmath>
#include <memory>

namespace
{

const auto angularDifference = [](float from, float to) {
    return glm::mod(to -from + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
};

}

namespace vv::test
{

class KeyboardMovementControllerTest : public ::testing::Test
{
public:
    static constexpr float dt{ 0.1f }; ///< dt is equal to 100ms

protected:
    void SetUp() override
    {
        mockInput = std::make_unique<MockInputHandler>();
        obj = Object();
    }

    std::unique_ptr<MockInputHandler> mockInput;
    Object obj;
};

TEST_F(KeyboardMovementControllerTest, MoveForward)
{
    const float yaw{ obj.transform.rotation.y };
    const glm::vec3 forward{ std::sin(yaw), 0.f, std::cos(yaw) };

    auto oldPos{ obj.transform.translation };

    mockInput->setKeyPressed(GLFW_KEY_W, true);
    KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

    auto newPos{ obj.transform.translation };
    auto delta{ newPos - oldPos };

    EXPECT_GT(glm::dot(delta, forward), 0.f);
}

TEST_F(KeyboardMovementControllerTest, MoveBackward)
{
    const float yaw{ obj.transform.rotation.y };
    const glm::vec3 forward{ std::sin(yaw), 0.f, std::cos(yaw) };

    auto oldPos{ obj.transform.translation };

    mockInput->setKeyPressed(GLFW_KEY_S, true);
    KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

    auto newPos{ obj.transform.translation };
    auto delta{ newPos - oldPos };

    EXPECT_LT(glm::dot(delta, forward), 0.f);
}

TEST_F(KeyboardMovementControllerTest, MoveRight)
{
    const float yaw{ obj.transform.rotation.y };
    const glm::vec3 forward{ std::sin(yaw), 0.f, std::cos(yaw) };
    const glm::vec3 right{ forward.z, 0.f, -forward.x };

    auto oldPos{ obj.transform.translation };

    mockInput->setKeyPressed(GLFW_KEY_D, true);
    KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

    auto newPos{ obj.transform.translation };
    auto delta{ newPos - oldPos };

    EXPECT_GT(glm::dot(delta, right), 0.f);
}

TEST_F(KeyboardMovementControllerTest, MoveLeft)
{
    const float yaw{ obj.transform.rotation.y };
    const glm::vec3 forward{ std::sin(yaw), 0.f, std::cos(yaw) };
    const glm::vec3 right{ forward.z, 0.f, -forward.x };

    auto oldPos{ obj.transform.translation };

    mockInput->setKeyPressed(GLFW_KEY_A, true);
    KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

    auto newPos{ obj.transform.translation };
    auto delta{ newPos - oldPos };

    EXPECT_LT(glm::dot(delta, right), 0.f);
}

TEST_F(KeyboardMovementControllerTest, MoveUp)
{
    const glm::vec3 up{ 0.f, -1.f, 0.f };
    auto oldPos{ obj.transform.translation };

    mockInput->setKeyPressed(GLFW_KEY_E, true);
    KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

    auto newPos{ obj.transform.translation };
    auto delta{ newPos - oldPos };

    EXPECT_GT(glm::dot(delta, up), 0.f);
}

TEST_F(KeyboardMovementControllerTest, MoveDown)
{
    const glm::vec3 up{ 0.f, -1.f, 0.f };
    auto oldPos{ obj.transform.translation };

    mockInput->setKeyPressed(GLFW_KEY_Q, true);
    KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

    auto newPos{ obj.transform.translation };
    auto delta{ newPos - oldPos };

    EXPECT_LT(glm::dot(delta, up), 0.f);
}

TEST_F(KeyboardMovementControllerTest, LookRight)
{
    const float oldYaw{ obj.transform.rotation.y };

    mockInput->setKeyPressed(GLFW_KEY_RIGHT, true);
    KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

    const float newYaw{ obj.transform.rotation.y };
    const float delta{ ::angularDifference(oldYaw, newYaw) };

    EXPECT_GT(delta, 0.f);
}

TEST_F(KeyboardMovementControllerTest, LookLeft)
{
    const float oldYaw{ obj.transform.rotation.y };

    mockInput->setKeyPressed(GLFW_KEY_LEFT, true);
    KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

    const float newYaw{ obj.transform.rotation.y };
    const float delta{ ::angularDifference(oldYaw, newYaw) };

    EXPECT_LT(delta, 0.f);
}

TEST_F(KeyboardMovementControllerTest, LookUp)
{
    const float oldYaw{ obj.transform.rotation.x };

    mockInput->setKeyPressed(GLFW_KEY_UP, true);
    KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

    const float newYaw{ obj.transform.rotation.x };
    const float delta{ ::angularDifference(oldYaw, newYaw) };

    EXPECT_GT(delta, 0.f);
}

TEST_F(KeyboardMovementControllerTest, LookDown)
{
    const float oldYaw{ obj.transform.rotation.x };
    
    mockInput->setKeyPressed(GLFW_KEY_DOWN, true);
    KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

    const float newYaw{ obj.transform.rotation.x };
    const float delta{ ::angularDifference(oldYaw, newYaw) };

    EXPECT_LT(delta, 0.f);
}

TEST_F(KeyboardMovementControllerTest, DiagonalMovementForwardRight)
{
    mockInput->setKeyPressed(GLFW_KEY_W, true);
    mockInput->setKeyPressed(GLFW_KEY_D, true);

    const float yaw{ obj.transform.rotation.y };
    const glm::vec3 forward{ std::sin(yaw), 0.f, std::cos(yaw) };
    const glm::vec3 right{ forward.z, 0.f, -forward.x };

    auto oldPosForward{ obj.transform.translation };
    auto oldPosRight{ obj.transform.translation };

    KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

    auto newPosForward{ obj.transform.translation };
    auto deltaForward{ newPosForward - oldPosForward };

    auto newPosRight{ obj.transform.translation };
    auto deltaRight{ newPosRight - oldPosRight };

    EXPECT_GT(glm::dot(deltaForward, forward), 0.f);
    EXPECT_GT(glm::dot(deltaRight, right), 0.f);
}

TEST_F(KeyboardMovementControllerTest, RotationAffectsMovementDirection)
{
    mockInput->setKeyPressed(GLFW_KEY_W, true);
    obj.transform.rotation.y = glm::half_pi<float>(); // NOTE: rotate object 90 degrees to the right

    KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

    EXPECT_GT(obj.transform.translation.x, 0.f);
    EXPECT_NEAR(obj.transform.translation.z, 0.f, 0.01f);
}

TEST_F(KeyboardMovementControllerTest, PitchClampingUp)
{
    mockInput->setKeyPressed(GLFW_KEY_UP, true);

    constexpr int ITERATIONS{ 100 };
    constexpr auto MAX_CLAMP_DEGREES{ glm::degrees(KeyboardMovementController::getClampMax()) };
    constexpr auto MIN_CLAMP_DEGREES{ glm::degrees(KeyboardMovementController::getClampMin()) };

    for(int i{ 0 }; i < ITERATIONS; ++i)
    {
        KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

        EXPECT_LE(obj.transform.rotation.x, MAX_CLAMP_DEGREES);
        EXPECT_GE(obj.transform.rotation.x, MIN_CLAMP_DEGREES);
    }
}

TEST_F(KeyboardMovementControllerTest, PitchClampingDown)
{
    mockInput->setKeyPressed(GLFW_KEY_DOWN, true);

    constexpr int ITERATIONS{ 100 };
    constexpr auto MAX_CLAMP_DEGREES{ glm::degrees(KeyboardMovementController::getClampMax()) };
    constexpr auto MIN_CLAMP_DEGREES{ glm::degrees(KeyboardMovementController::getClampMin()) };

    for(int i{ 0 }; i < ITERATIONS; ++i)
    {
        KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

        EXPECT_LE(obj.transform.rotation.x, MAX_CLAMP_DEGREES);
        EXPECT_GE(obj.transform.rotation.x, MIN_CLAMP_DEGREES);
    }
}

TEST_F(KeyboardMovementControllerTest, YawWrapping)
{
    mockInput->setKeyPressed(GLFW_KEY_RIGHT, true);

    constexpr int ITERATIONS{ 100 };
    for(int i{ 0 }; i < ITERATIONS; ++i)
    {
        KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

        EXPECT_GE(obj.transform.rotation.y, 0);
        EXPECT_LT(obj.transform.rotation.x, glm::two_pi<float>());
    }
}

TEST_F(KeyboardMovementControllerTest, QuitKeySignalsCloseWindow)
{
    mockInput->setKeyPressed(GLFW_KEY_ESCAPE, true);

    ASSERT_FALSE(mockInput->shouldClose());

    KeyboardMovementController::moveInPlaneXZ(*mockInput, dt, obj);

    ASSERT_TRUE(mockInput->shouldClose());
}

} // namespace vv::test
