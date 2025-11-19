#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_KEYBOARD_MOVEMENT_CONTROLLER_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_KEYBOARD_MOVEMENT_CONTROLLER_HPP

#include "utility/Object.hpp"

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

    /// \brief Move an object based on key input
    ///
    /// \param window GLFWwindow pointer to get keyboard input
    /// \param dt delta time
    /// \param object the object that the input transformation is applied to
    void moveInPlaneXZ(GLFWwindow* window, float dt, Object& object);

private:
    static constexpr float moveSpeed{ 3.f };
    static constexpr float lookSpeed{ 1.f };
    static constexpr KeyMappings keys{};
};

} //!vv

#endif //!VULKAN_VOXELS_SRC_ENGINE_UTILITY_KEYBOARD_MOVEMENT_CONTROLLER_HPP
