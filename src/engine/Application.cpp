#include "Application.hpp"

#include "renderSystems/BasicRenderSystem.hpp"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"
#include "utility/Camera.hpp"
#include "utility/KeyboardMovementController.hpp"
#include "utility/Object.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/ext/vector_float3.hpp"
#include "GLFW/glfw3.h"
#include <vulkan/vulkan_core.h>

#include <chrono>
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
    KeyboardMovementController cameraController{};
    Camera camera{};
    Object viewer{};

    auto currentTime{ std::chrono::high_resolution_clock::now() };

    while(!m_window.shouldClose())
    {
        glfwPollEvents();

        auto newTime{ std::chrono::high_resolution_clock::now() };
        float dt{ std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count() };
        currentTime = newTime;

        cameraController.moveInPlaneXZ(m_window.getHandle(), dt, viewer);
        camera.setViewXYZ(viewer.transform.translation, viewer.transform.rotation);

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
    Model::Builder builder{};
    builder.vertices = {
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}}
    };

    for(auto& v : builder.vertices)
        v.position += offset;

    builder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
        12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

    return std::make_unique<Model>(device, builder);
}

} // !vv
