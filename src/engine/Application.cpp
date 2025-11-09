#include "Application.hpp"
#include "GLFW/glfw3.h"

namespace vv
{

void Application::run()
{
    while(!m_window.shouldClose())
    {
        glfwPollEvents();
    }
}

} // !vv
