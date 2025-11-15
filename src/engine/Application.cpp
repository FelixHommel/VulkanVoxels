#include "Application.hpp"

#include "Object.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "GLFW/glfw3.h"

#include <array>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vv
{

Application::Application()
{
    loadObjects();
    createPipelineLayout();
    recreateSwapchain();
    createCommandBuffers();
}

Application::~Application()
{
    vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void Application::run()
{
    while(!m_window.shouldClose())
    {
        glfwPollEvents();
        drawFrame();
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

void Application::createPipelineLayout()
{
    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(SimplePushConstantData)
    };

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = 0;
    createInfo.pSetLayouts = nullptr;
    createInfo.pushConstantRangeCount = 1;
    createInfo.pPushConstantRanges = &pushConstantRange;

    if(vkCreatePipelineLayout(m_device.device(), &createInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout");
}

void Application::createPipeline()
{
#if defined(VV_ENABLE_ASSERTS)
    assert(m_swapchain != nullptr && "Cannot create pipeline without swapchain");
    assert(m_pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline without pipeline layout");
#endif

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = m_swapchain->getRenderPass();
    pipelineConfig.pipelineLayout = m_pipelineLayout;

    m_pipeline = std::make_unique<Pipeline>(m_device, VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH, pipelineConfig);
}

void Application::createCommandBuffers()
{
    m_commandBufers.resize(m_swapchain->imageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_device.commandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<std::uint32_t>(m_commandBufers.size());
    
    if(vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBufers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers");
}

void Application::freeCommandBuffers()
{
    vkFreeCommandBuffers(m_device.device(), m_device.commandPool(), static_cast<std::uint32_t>(m_commandBufers.size()), m_commandBufers.data());
    m_commandBufers.clear();
}

void Application::recordCommandBuffer(std::size_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if(vkBeginCommandBuffer(m_commandBufers[imageIndex], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording to command buffer");

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { CLEAR_COLOR };
    clearValues[1].depthStencil = { .depth=1.f, .stencil=0 };

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = m_swapchain->getRenderPass();
    renderPassBeginInfo.framebuffer = m_swapchain->getFramebuffer(imageIndex);
    renderPassBeginInfo.renderArea = {
        .offset = { .x = 0, .y = 0 },
        .extent = m_swapchain->getExtent()
    };
    renderPassBeginInfo.clearValueCount = static_cast<std::uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_commandBufers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{
        .x = 0.f,
        .y = 0.f,
        .width = static_cast<float>(m_swapchain->getExtent().width),
        .height = static_cast<float>(m_swapchain->getExtent().height),
        .minDepth = 0.f,
        .maxDepth = 1.f
    };

    VkRect2D scissor{
        .offset = {
            .x = 0,
            .y = 0
        },
        .extent = m_swapchain->getExtent()
    };

    vkCmdSetViewport(m_commandBufers[imageIndex], 0, 1, &viewport);
    vkCmdSetScissor(m_commandBufers[imageIndex], 0, 1, &scissor);

    renderObjects(m_commandBufers[imageIndex]);

    vkCmdEndRenderPass(m_commandBufers[imageIndex]);

    if(vkEndCommandBuffer(m_commandBufers[imageIndex]) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer");
}

void Application::renderObjects(VkCommandBuffer commandBuffer)
{
    m_pipeline->bind(commandBuffer);

    for(const auto& obj : m_objects)
    {
        SimplePushConstantData pushData{
            .transform = obj.transform2d.mat2(),
            .offset = obj.transform2d.translation,
            .color = obj.color
        };

        vkCmdPushConstants(commandBuffer,
            m_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &pushData);

        obj.model->bind(commandBuffer);
        obj.model->draw(commandBuffer);
    }
}

void Application::drawFrame()
{
    std::uint32_t imageIndex{};
    auto result{ m_swapchain->acquireNextImage(&imageIndex) };

    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return;
    }

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swapchain image");

    recordCommandBuffer(imageIndex);
    result = m_swapchain->submitCommandBuffer(&m_commandBufers[imageIndex], &imageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized())
    {
        m_window.resetWindowResizeFlag();
        recreateSwapchain();
        return;
    }

    if(result != VK_SUCCESS)
        throw std::runtime_error("failed to present swapchain image");
}

void Application::recreateSwapchain()
{
    auto extent{ m_window.getExtent() };
    while(extent.width == 0 || extent.height == 0)
    {
        extent = m_window.getExtent();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(m_device.device());

    if(m_swapchain == nullptr)
        m_swapchain = std::make_unique<Swapchain>(m_device, extent);
    else
    {
        m_swapchain = std::make_unique<Swapchain>(m_device, extent, std::move(m_swapchain));
        freeCommandBuffers();
        createCommandBuffers();
    }

    createPipeline();
}

} // !vv
