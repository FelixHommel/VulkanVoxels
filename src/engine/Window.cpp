#include "Window.hpp"

#include "GLFW/glfw3.h"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"

#include <cstdint>
#include <string>

namespace vv
{

Window::Window(std::uint16_t width, std::uint16_t height, const std::string& title)
    : m_width{ width }
    , m_height{ height }
    , m_title{ title }
{
    if(glfwInit() != GLFW_TRUE)
        spdlog::log(spdlog::level::err, "Failed to create the window");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

}
