#include "Application.hpp"

#include "glm/ext/vector_float3.hpp"
#include "renderSystems/BasicRenderSystem.hpp"
#include "utility/Camera.hpp"
#include "utility/Object.hpp"

#include "GLFW/glfw3.h"
#include <vulkan/vulkan_core.h>

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
    Camera camera{};
    camera.setViewTarget(glm::vec3{ -1.f, -2.f, -2.f }, glm::vec3{ 0.f, 0.f, 2.5f });

    while(!m_window.shouldClose())
    {
        glfwPollEvents();

        const float aspectRatio{ m_renderer.getAspectRatio() };
        camera.setPerspectiveProjection(glm::radians(50.f), aspectRatio, 0.1f, 10.f);

        if (const auto commandBuffer{ m_renderer.beginFrame() })
        {
            m_renderer.beginRenderPass(commandBuffer);

            basicRenderSystem.renderObjects(commandBuffer, m_objects, camera);

            m_renderer.endRenderPass(commandBuffer);
            m_renderer.endFrame();
        }
    }

    vkDeviceWaitIdle(m_device.device());
}

/// \brief Load all objects that are being used
void Application::loadObjects()
{
    const std::shared_ptr<Model> model{ loadCubeModel(m_device, {0.f, 0.f, 0.f}) };
    Object cube{};
    cube.model = model;
    cube.transform.translation = { 0.f, 0.f, 2.5f };
    cube.transform.scale = { 0.5f, 0.5f, 0.5f };
    m_objects.push_back(std::move(cube));
}

/// \brief Create a Cube Model
///
/// \param device \ref Device used to allocate memory for the model
/// \param offset an offset applied to the model on a per-vertex basis
std::unique_ptr<Model> Application::loadCubeModel(Device& device, const glm::vec3& offset)
{
    std::vector<Model::Vertex> vertices{
        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

        // top face (orange)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}}
    };

    for(auto& [position, color] : vertices)
        position += offset;

    return std::make_unique<Model>(device, vertices);
}

} // !vv
