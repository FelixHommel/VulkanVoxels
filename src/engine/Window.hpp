#ifndef SRC_ENGINE_WINDOW_HPP
#define SRC_ENGINE_WINDOW_HPP

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
    Window(std::uint16_t width, std::uint16_t height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    /// \brief Returns whether the window should be closed or not
    ///
    /// \return  *true* if the window should close, *false* if not
    [[nodiscard]] bool shouldClose() const { return glfwWindowShouldClose(m_window) != 0; }

private:
    std::uint16_t m_width;
    std::uint16_t m_height;
    std::string m_title;
    GLFWwindow* m_window;
};

} //!vv

#endif //!SRC_ENGINE_WINDOW_HPP
