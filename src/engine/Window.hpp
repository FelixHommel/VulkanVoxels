#ifndef SRC_ENGINE_WINDOW_HPP
#define SRC_ENGINE_WINDOW_HPP

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <cstdint>
#include <string>

namespace vv
{

/// \brief The Window class is a wrapper around the GLFW windoing utility
///
/// \author Felix Hommel
/// \date 11/9/2025
class Window
{
public:
    Window(std::uint32_t width, std::uint32_t height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    /// \brief Returns whether the window should be closed or not
    ///
    /// \return  *true* if the window should close, *false* if not
    [[nodiscard]] bool shouldClose() const { return glfwWindowShouldClose(m_window) != 0; }
    [[nodiscard]] VkExtent2D getExtent() const noexcept { return { .width = m_width, .height = m_height }; }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
    std::uint32_t m_width;
    std::uint32_t m_height;
    std::string m_title;
    GLFWwindow* m_window;
};

} //!vv

#endif //!SRC_ENGINE_WINDOW_HPP
