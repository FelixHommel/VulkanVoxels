#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_GLFW_INPUT_HANDLER_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_GLFW_INPUT_HANDLER_HPP

#include "GLFW/glfw3.h"
#include "utility/IInputHandler.hpp"

namespace vv
{

class GLFWInputHandler : public IInputHandler
{
public:
    explicit GLFWInputHandler(GLFWwindow* window) : m_window{ window } {}

    [[nodiscard]] bool isKeyPressed(int key) const override { return glfwGetKey(m_window, key) == GLFW_PRESS; }

    [[nodiscard]] bool shouldClose() const override { return glfwWindowShouldClose(m_window) != GLFW_FALSE; }

    void setShouldClose(bool close) override { glfwSetWindowShouldClose(m_window, static_cast<int>(close)); }

private:
    GLFWwindow* m_window;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_GLFW_INPUT_HANDLER_HPP
