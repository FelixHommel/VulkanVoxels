#include "Application.hpp"

#include "BasicRenderSystem.hpp"
#include "Object.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "GLFW/glfw3.h"
#include <vulkan/vulkan_core.h>

#include <cassert>
#include <memory>
#include <vector>

namespace vv
{

Application::Application()
{
    loadObjects();
}

void Application::run()
{
    BasicRenderSystem basicRenderSystem{ m_device, m_renderer.getRenderPass() };

    while(!m_window.shouldClose())
    {
        glfwPollEvents();

        if(auto commandBuffer{ m_renderer.beginFrame() })
        {
            m_renderer.beginRenderPass(commandBuffer);

            basicRenderSystem.renderObjects(commandBuffer, m_objects);

            m_renderer.endRenderPass(commandBuffer);
            m_renderer.endFrame();
        }
    }

    vkDeviceWaitIdle(m_device.device());
}

void Application::loadObjects()
{
    std::vector<Model::Vertex> vertices{
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };
    auto model{ std::make_shared<Model>(m_device, vertices) };

    Object triangle{ Object::createObject() };
    triangle.model = model;
    triangle.color = { .1f, .8f, .1f };
    triangle.transform2d.translation.x = .2f;
    triangle.transform2d.scale = { 2.f, .5f };
    triangle.transform2d.rotation = .25f * glm::two_pi<float>();

    m_objects.push_back(std::move(triangle));
}

} // !vv
