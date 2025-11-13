#include "Window.hpp"

#include "GLFW/glfw3.h"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"

#include <cstdint>
#include <stdexcept>
#include <string>

namespace vv
{

Window::Window(std::uint32_t width, std::uint32_t height, const std::string& title)
    : m_width{ width }
    , m_height{ height }
    , m_title{ title }
{
    if(glfwInit() != GLFW_TRUE)
        spdlog::log(spdlog::level::err, "Failed to create the window");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(static_cast<int>(m_width), static_cast<int>(m_height), m_title.c_str(), nullptr, nullptr);
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if(glfwCreateWindowSurface(instance, m_window, nullptr, surface) != 0)
        throw std::runtime_error("Failed to create window surface");
}

}
