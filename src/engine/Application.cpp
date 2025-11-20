#include "Application.hpp"

#include "renderSystems/BasicRenderSystem.hpp"
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
    std::shared_ptr<Model> model{ Model::loadFromFile(m_device, FLAT_VASE_PATH) };
    Object flatVase{};
    flatVase.model = model;
    flatVase.transform.translation = { -0.5f, 0.5f, 2.5f };
    flatVase.transform.scale = glm::vec3{ 3.f, 1.5f, 3.f };

    model = Model::loadFromFile(m_device, SMOOTH_VASE_PATH);
    Object smoothVase{};
    smoothVase.model = model;
    smoothVase.transform.translation = { 0.5f, 0.5f, 2.5f };
    smoothVase.transform.scale = glm::vec3{ 3.f, 1.f, 3.f };

    m_objects.push_back(std::move(flatVase));
    m_objects.push_back(std::move(smoothVase));
}

} // !vv
