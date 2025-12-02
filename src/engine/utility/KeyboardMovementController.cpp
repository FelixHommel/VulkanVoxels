#include "KeyboardMovementController.hpp"

#include "utility/Object.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "GLFW/glfw3.h"
#include "glm/common.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/constants.hpp"

#include <cmath>
#include <limits>

namespace vv
{

void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, Object& object)
{
	if(glfwGetKey(window, keys.quit) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	glm::vec3 rotate{ 0.f };
	if(glfwGetKey(window, keys.lookRight) == GLFW_PRESS)
		rotate.y += 1.f;
	if(glfwGetKey(window, keys.lookLeft) == GLFW_PRESS)
		rotate.y -= 1.f;
	if(glfwGetKey(window, keys.lookUp) == GLFW_PRESS)
		rotate.x += 1.f;
	if(glfwGetKey(window, keys.lookDown) == GLFW_PRESS)
		rotate.x -= 1.f;

	// NOTE: Ensure that rotate is not 0
	if(glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
		object.transform.rotation += lookSpeed * dt * glm::normalize(rotate);

	// NOTE: Limit pitch values between about +/- ~85 degrees
	object.transform.rotation.x = glm::clamp(object.transform.rotation.x, ROTATION_CLAMP_MIN, ROTATION_CLAMP_MAX);
	object.transform.rotation.y = glm::mod(object.transform.rotation.y, glm::two_pi<float>());

	const float yaw{ object.transform.rotation.y };
	const glm::vec3 forward{ std::sin(yaw), 0.f, std::cos(yaw) };
	const glm::vec3 right{ forward.z, 0.f, -forward.x };
	const glm::vec3 up{ 0.f, -1.f, 0.f };

	glm::vec3 move{ 0.f };
	if(glfwGetKey(window, keys.moveForward) == GLFW_PRESS)
		move += forward;
	if(glfwGetKey(window, keys.moveBackward) == GLFW_PRESS)
		move -= forward;
	if(glfwGetKey(window, keys.moveRight) == GLFW_PRESS)
		move += right;
	if(glfwGetKey(window, keys.moveLeft) == GLFW_PRESS)
		move -= right;
	if(glfwGetKey(window, keys.moveUp) == GLFW_PRESS)
		move += up;
	if(glfwGetKey(window, keys.moveDown) == GLFW_PRESS)
		move -= up;

	// NOTE: Ensure that move is not 0
	if(glm::dot(move, move) > std::numeric_limits<float>::epsilon())
		object.transform.translation += moveSpeed * dt * glm::normalize(move);
}

} // namespace vv
