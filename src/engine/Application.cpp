#include "Application.hpp"

#include "GLFW/glfw3.h"

#include <array>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace vv
{

Application::Application()
{
    createPipelineLayout();
    createPipeline();
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

void Application::createPipelineLayout()
{
    VkPipelineLayoutCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    if(vkCreatePipelineLayout(m_device.device(), &createInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout");
}

void Application::createPipeline()
{
    auto pipelineConfig{ Pipeline::defaultPipelineConfigInfo(m_swapchain.getExtent().width, m_swapchain.getExtent().height) };
    pipelineConfig.renderPass = m_swapchain.getRenderPass();
    pipelineConfig.pipelineLayout = m_pipelineLayout;

    m_pipeline = std::make_unique<Pipeline>(m_device, VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH, pipelineConfig);
}

void Application::createCommandBuffers()
{
    m_commandBufers.resize(m_swapchain.imageCount());

    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_device.commandPool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<std::uint32_t>(m_commandBufers.size())
    };
    
    if(vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBufers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers");

    for(std::size_t i{ 0 }; i < m_commandBufers.size(); ++i)
    {
        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };

        if(vkBeginCommandBuffer(m_commandBufers[i], &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording to command buffer");

        std::array<VkClearValue, 2> clearValues{{
            {
                .color = { 0.1f, 0.1f, 0.1f, 1.f }
            },
            {
                .depthStencil = { 1.f, 0 }
            }
        }};
        VkRenderPassBeginInfo renderPassBeginInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = m_swapchain.getRenderPass(),
            .framebuffer = m_swapchain.getFramebuffer(i),
            .renderArea = {
                .offset = { .x = 0, .y = 0 },
                .extent = m_swapchain.getExtent()
            },
            .clearValueCount = static_cast<std::uint32_t>(clearValues.size()),
            .pClearValues = clearValues.data()
        };

        vkCmdBeginRenderPass(m_commandBufers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        m_pipeline->bind(m_commandBufers[i]);
        vkCmdDraw(m_commandBufers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(m_commandBufers[i]);

        if(vkEndCommandBuffer(m_commandBufers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer");
    }
}

void Application::drawFrame()
{
    std::uint32_t imageIndex{};
    auto result{ m_swapchain.acquireNextImage(&imageIndex) };

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swapchain image");

    result = m_swapchain.submitCommandBuffer(&m_commandBufers[imageIndex], &imageIndex);

    if(result != VK_SUCCESS)
        throw std::runtime_error("failed to present swapchain image");
}

} // !vv
