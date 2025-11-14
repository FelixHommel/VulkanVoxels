#include "Application.hpp"

#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

#include <array>
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vv
{

Application::Application()
{
    loadModels();
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

void Application::loadModels()
{
    std::vector<Model::Vertex> vertices{};
    createSierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});
    m_model = std::make_unique<Model>(m_device, vertices);
}

void Application::createPipelineLayout()
{
    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = 0;
    createInfo.pSetLayouts = nullptr;
    createInfo.pushConstantRangeCount = 0;
    createInfo.pPushConstantRanges = nullptr;

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

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_device.commandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<std::uint32_t>(m_commandBufers.size());
    
    if(vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBufers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers");

    for(std::size_t i{ 0 }; i < m_commandBufers.size(); ++i)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if(vkBeginCommandBuffer(m_commandBufers[i], &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording to command buffer");

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { CLEAR_COLOR };
        clearValues[1].depthStencil = { .depth=1.f, .stencil=0 };

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = m_swapchain.getRenderPass();
        renderPassBeginInfo.framebuffer = m_swapchain.getFramebuffer(i);
        renderPassBeginInfo.renderArea = {
            .offset = { .x = 0, .y = 0 },
            .extent = m_swapchain.getExtent()
        };
        renderPassBeginInfo.clearValueCount = static_cast<std::uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_commandBufers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        m_pipeline->bind(m_commandBufers[i]);
        m_model->bind(m_commandBufers[i]);
        m_model->draw(m_commandBufers[i]);

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

void Application::createSierpinski(std::vector<Model::Vertex>& vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top)
{
    if(depth <= 0)
    {
        vertices.push_back({ top });
        vertices.push_back({ right });
        vertices.push_back({ left });
    }
    else
    {
        auto leftTop{ 0.5f * (left + top) };
        auto rightTop{ 0.5f * (right + top) };
        auto leftRight{ 0.5f * (left + right) };

        createSierpinski(vertices, depth - 1, left, leftRight, leftTop);
        createSierpinski(vertices, depth - 1, leftRight, right, rightTop);
        createSierpinski(vertices, depth - 1, leftTop, rightTop, top);
    }
}

} // !vv
