#include "PointLightRenderSystem.hpp"

#include "core/Device.hpp"
#include "core/Pipeline.hpp"
#include "utility/FrameInfo.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vulkan/vulkan_core.h>

#include <cassert>
#include <memory>
#include <stdexcept>
#include <vector>

namespace vv
{

PointLightRenderSystem::PointLightRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    : device(device)
{
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

PointLightRenderSystem::~PointLightRenderSystem()
{
    vkDestroyPipelineLayout(device.device(), m_pipelineLayout, nullptr);
}

void PointLightRenderSystem::render(FrameInfo& frameInfo) const
{
    m_pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipelineLayout,
        0,
        1,
        &frameInfo.gloablDescriptorSet,
        0,
        nullptr);


    constexpr std::uint32_t squareVertexCount{ 6 };
    vkCmdDraw(frameInfo.commandBuffer, squareVertexCount, 1, 0, 0);
}

/// \brief Create a PipelineLayout that can be used to create a Pipeline
void PointLightRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = static_cast<std::uint32_t>(descriptorSetLayouts.size());
    createInfo.pSetLayouts = descriptorSetLayouts.data();
    createInfo.pushConstantRangeCount = 0;
    createInfo.pPushConstantRanges = nullptr;

    if(vkCreatePipelineLayout(device.device(), &createInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout");
}

/// \brief Create a Pipeline for Rendering
void PointLightRenderSystem::createPipeline(VkRenderPass renderPass)
{
#if defined(VV_ENABLE_ASSERTS)
    assert(m_pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline without pipeline layout");
#endif

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.bindingDescription.clear();
    pipelineConfig.attributeDescription.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;

    m_pipeline = std::make_unique<Pipeline>(device, VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH, pipelineConfig);
}

} // !vv
