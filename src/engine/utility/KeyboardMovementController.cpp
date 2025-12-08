#include "KeyboardMovementController.hpp"

#include "object/Object.hpp"
#include "utility/GLFWInputHandler.hpp"
#include "utility/IInputHandler.hpp"
#include "utility/object/components/TransformComponent.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "GLFW/glfw3.h"
#include "glm/common.hpp"
#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

#include <cmath>
#include <limits>

namespace vv
{

void KeyboardMovementController::moveInPlaneXZ(IInputHandler& input, float dt, Object& object)
{
    if(input.isKeyPressed(KEYS.quit))
        input.setShouldClose(true);

    glm::vec3 rotate(0.f);
    if(input.isKeyPressed(KEYS.lookRight))
        rotate.y += 1.f;
    if(input.isKeyPressed(KEYS.lookLeft))
        rotate.y -= 1.f;
    if(input.isKeyPressed(KEYS.lookUp))
        rotate.x += 1.f;
    if(input.isKeyPressed(KEYS.lookDown))
        rotate.x -= 1.f;

    // NOTE: Ensure that rotate is not 0
    if(glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
        object.getComponent<TransformComponent>()->rotation += LOOK_SPEED * dt * glm::normalize(rotate);

    // NOTE: Limit pitch values between about +/- ~85 degrees
    object.getComponent<TransformComponent>()->rotation.x = glm::clamp(object.getComponent<TransformComponent>()->rotation.x, ROTATION_CLAMP_MIN, ROTATION_CLAMP_MAX);
    object.getComponent<TransformComponent>()->rotation.y = glm::mod(object.getComponent<TransformComponent>()->rotation.y, glm::two_pi<float>());

    const float yaw{ object.getComponent<TransformComponent>()->rotation.y };
    const glm::vec3 forward{ std::sin(yaw), 0.f, std::cos(yaw) };
    const glm::vec3 right{ forward.z, 0.f, -forward.x };
    constexpr glm::vec3 up{ 0.f, -1.f, 0.f };

    glm::vec3 move{ 0.f };
    if(input.isKeyPressed(KEYS.moveForward))
        move += forward;
    if(input.isKeyPressed(KEYS.moveBackward))
        move -= forward;
    if(input.isKeyPressed(KEYS.moveRight))
        move += right;
    if(input.isKeyPressed(KEYS.moveLeft))
        move -= right;
    if(input.isKeyPressed(KEYS.moveUp))
        move += up;
    if(input.isKeyPressed(KEYS.moveDown))
        move -= up;

    // NOTE: Ensure that move is not 0
    if(glm::dot(move, move) > std::numeric_limits<float>::epsilon())
        object.getComponent<TransformComponent>()->translation += MOVE_SPEED * dt * glm::normalize(move);
}

void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, Object& object)
{
    GLFWInputHandler input(window);
    moveInPlaneXZ(input, dt, object);
}

} // namespace vv
