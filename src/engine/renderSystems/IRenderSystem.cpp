#include "IRenderSystem.hpp"

#include "core/GraphicsPipeline.hpp"
#include "utility/exceptions/VulkanException.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

namespace vv
{

/// \brief Create a PipelineLayout that can be used to create a Pipeline
void IRenderSystem::createGraphicsPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    constexpr VkPushConstantRange pushConstantRange{ .stageFlags
                                                     = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                                     .offset = 0,
                                                     .size = sizeof(SimplePushConstantData) };
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = static_cast<std::uint32_t>(descriptorSetLayouts.size());
    createInfo.pSetLayouts = descriptorSetLayouts.data();
    createInfo.pushConstantRangeCount = 1;
    createInfo.pPushConstantRanges = &pushConstantRange;

    const VkResult result{ vkCreatePipelineLayout(device->device(), &createInfo, nullptr, &m_graphicsPipelineLayout) };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to create pipeline layout", result);
}

/// \brief Create a Pipeline for Rendering
void IRenderSystem::createGraphicsPipeline(
    VkRenderPass renderPass,
    const std::filesystem::path& vertexShaderPath,
    const std::filesystem::path& fragmentShaderPath
)
{
#if defined(VV_ENABLE_ASSERTS)
    assert(m_graphicsPipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline without pipeline layout");
#endif

    GraphicsPipelineConfigInfo pipelineConfig{};
    GraphicsPipeline::defaultGraphicsPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_graphicsPipelineLayout;

    m_graphicsPipeline
        = std::make_unique<GraphicsPipeline>(device, vertexShaderPath, fragmentShaderPath, pipelineConfig);
}

} // namespace vv
