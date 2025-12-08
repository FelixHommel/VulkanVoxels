#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_KEYBOARD_MOVEMENT_CONTROLLER_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_KEYBOARD_MOVEMENT_CONTROLLER_HPP

#include "utility/IInputHandler.hpp"
#include "object/Object.hpp"

#include "GLFW/glfw3.h"

namespace vv
{

/// \brief Simple struct containing a map that assigns each action a GLFW event
///
/// \author Felix Hommel
/// \date 11/19/2025
struct KeyMappings
{
    int moveLeft{ GLFW_KEY_A };
    int moveRight{ GLFW_KEY_D };
    int moveForward{ GLFW_KEY_W };
    int moveBackward{ GLFW_KEY_S };
    int moveUp{ GLFW_KEY_E };
    int moveDown{ GLFW_KEY_Q };
    int lookLeft{ GLFW_KEY_LEFT };
    int lookRight{ GLFW_KEY_RIGHT };
    int lookUp{ GLFW_KEY_UP };
    int lookDown{ GLFW_KEY_DOWN };

    int quit{ GLFW_KEY_ESCAPE };
};

// \brief Handle user input and apply to an object
//
// \author Felix Hommel
// \date 11/19/2025
class KeyboardMovementController
{
public:
    KeyboardMovementController() = default;
    ~KeyboardMovementController() = default;

    KeyboardMovementController(const KeyboardMovementController&) = default;
    KeyboardMovementController(KeyboardMovementController&&) = delete;
    KeyboardMovementController& operator=(const KeyboardMovementController&) = default;
    KeyboardMovementController& operator=(KeyboardMovementController&&) = delete;

    static constexpr float getClampMin() noexcept { return ROTATION_CLAMP_MIN; }
    static constexpr float getClampMax() noexcept { return ROTATION_CLAMP_MAX; }

    /// \brief Move an object based on key input
    ///
    /// \param window GLFWwindow pointer to get keyboard input
    /// \param dt delta time
    /// \param object the object that the input transformation is applied to
    static void moveInPlaneXZ(IInputHandler& input, float dt, Object& object);
    /// \brief Move an object based on key input
    ///
    /// \param window GLFWwindow pointer to get keyboard input
    /// \param dt delta time
    /// \param object the object that the input transformation is applied to
    static void moveInPlaneXZ(GLFWwindow* window, float dt, Object& object);

private:
    static constexpr float MOVE_SPEED{ 3.f };
    static constexpr float LOOK_SPEED{ 1.f };
    static constexpr float ROTATION_CLAMP_MIN{ -1.5f };
    static constexpr float ROTATION_CLAMP_MAX{ 1.5f };
    static constexpr KeyMappings KEYS{};
};

} // namespace vv

#endif //! VULKAN_VOXELS_SRC_ENGINE_UTILITY_KEYBOARD_MOVEMENT_CONTROLLER_HPP
