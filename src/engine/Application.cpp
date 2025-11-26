#include "Application.hpp"

#include "core/Buffer.hpp"
#include "core/DescriptorPool.hpp"
#include "core/DescriptorSetLayout.hpp"
#include "core/DescriptorWriter.hpp"
#include "core/Swapchain.hpp"
#include "renderSystems/BasicRenderSystem.hpp"
#include "utility/Camera.hpp"
#include "utility/FrameInfo.hpp"
#include "utility/KeyboardMovementController.hpp"
#include "utility/Model.hpp"
#include "utility/Object.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "GLFW/glfw3.h"
#include "glm/ext/vector_float3.hpp"
#include <vulkan/vulkan_core.h>

#include <chrono>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace vv
{

Application::Application()
{
    m_globalPool =
        DescriptorPool::Builder(m_device)
            .setMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .build();

    loadObjects();
}

void Application::run()
{
    std::vector<std::unique_ptr<Buffer>> uboBuffers(Swapchain::MAX_FRAMES_IN_FLIGHT);
    for(std::size_t i{ 0 }; i < uboBuffers.size(); ++i)
    {
        uboBuffers[i] = std::make_unique<Buffer>(
            m_device,
            sizeof(GloablUBO),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            m_device.properties.limits.minUniformBufferOffsetAlignment
        );
        uboBuffers[i]->map();
    }

    auto globalSetLayout{
        DescriptorSetLayout::Builder(m_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .build()
    };

    std::vector<VkDescriptorSet> globalDescriptorSets(Swapchain::MAX_FRAMES_IN_FLIGHT);
    for(std::size_t i{ 0 }; i < globalDescriptorSets.size(); ++i)
    {
        auto bufferInfo{ uboBuffers[i]->descriptorInfo() };

        DescriptorWriter(*globalSetLayout, *m_globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }

    BasicRenderSystem basicRenderSystem{ m_device, m_renderer.getRenderPass(), globalSetLayout->getDescriptorLayout() };
    Camera camera{};
    Object viewer{};

    auto currentTime{ std::chrono::high_resolution_clock::now() };

    while(!m_window.shouldClose())
    {
        glfwPollEvents();

        auto newTime{ std::chrono::high_resolution_clock::now() };
        float dt{ std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count() };
        currentTime = newTime;

        KeyboardMovementController::moveInPlaneXZ(m_window.getHandle(), dt, viewer);
        camera.setViewXYZ(viewer.transform.translation, viewer.transform.rotation);

        const float aspectRatio{ m_renderer.getAspectRatio() };
        constexpr float fov{ 50.f };
        constexpr float nearPlane{ 0.1f };
        constexpr float farPlane{ 10.f };
        camera.setPerspectiveProjection(glm::radians(fov), aspectRatio, nearPlane, farPlane);

        if (auto* const commandBuffer{ m_renderer.beginFrame() })
        {
            std::size_t frameIndex{ m_renderer.getFrameIndex() };
            FrameInfo frameInfo{
                .frameIndex = frameIndex,
                .dt = dt,
                .commandBuffer = commandBuffer,
                .camera = camera,
                .gloablDescriptorSet = globalDescriptorSets[frameIndex]
            };
            GloablUBO ubo{};
            ubo.porjectionView = camera.getProjection() * camera.getView();

            uboBuffers[frameIndex]->writeToBuffer(ubo);
            uboBuffers[frameIndex]->flush();

            m_renderer.beginRenderPass(commandBuffer);

            basicRenderSystem.renderObjects(frameInfo, m_objects);

            m_renderer.endRenderPass(commandBuffer);
            m_renderer.endFrame();
        }
    }

    vkDeviceWaitIdle(m_device.device());
}

/// \brief Load all objects that are being used
void Application::loadObjects()
{
    constexpr glm::vec3 vaseScale{ glm::vec3{ 3.f, 1.5f, 3.f } };
    constexpr glm::vec3 flatVasePos{ glm::vec3{ -0.5f, 0.5f, 2.5f } };
    constexpr glm::vec3 smoothVasePos{ glm::vec3{ 0.5f, 0.5f, 2.5f } };

    std::shared_ptr<Model> model{ Model::loadFromFile(m_device, FLAT_VASE_PATH) };
    Object flatVase{};
    flatVase.model = model;
    flatVase.transform.translation = flatVasePos;
    flatVase.transform.scale = vaseScale;

    model = Model::loadFromFile(m_device, SMOOTH_VASE_PATH);
    Object smoothVase{};
    smoothVase.model = model;
    smoothVase.transform.translation = smoothVasePos;
    smoothVase.transform.scale = vaseScale;

    m_objects.push_back(std::move(flatVase));
    m_objects.push_back(std::move(smoothVase));
}

} // !vv
