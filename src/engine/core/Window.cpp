#include "Window.hpp"

#include "utility/exceptions/VulkanException.hpp"

#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <string>
#include <utility>

namespace vv
{

Window::Window(const std::uint32_t width, const std::uint32_t height, std::string title)
    : m_width{ width }, m_height{ height }, m_title{ std::move(title) }
{
    if(glfwInit() != GLFW_TRUE)
        spdlog::error("Failed to create the window");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window
        = glfwCreateWindow(static_cast<int>(m_width), static_cast<int>(m_height), m_title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);

    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const
{
    const VkResult result{ glfwCreateWindowSurface(instance, m_window, nullptr, surface) };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to create window surface", result);
}

/// \brief Callback function for when the window is resized
///
/// \param window pointer to a GLFW window
/// \param width the new width of the window
/// \param height the new height of the window
void Window::framebufferResizeCallback(GLFWwindow* window, const int width, const int height)
{
    auto* pWindow{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };

    pWindow->m_wasResized = true;
    pWindow->m_width = static_cast<std::uint32_t>(width);
    pWindow->m_height = static_cast<std::uint32_t>(height);
}

} // namespace vv
