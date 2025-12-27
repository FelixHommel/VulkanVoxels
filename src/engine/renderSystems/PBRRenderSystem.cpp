#include "PBRRenderSystem.hpp"

#include "core/Device.hpp"
#include "renderSystems/IRenderSystem.hpp"
#include "utility/FrameInfo.hpp"
#include "utility/exceptions/VulkanException.hpp"
#include "utility/material/Material.hpp"

#include <vector>
#include <vulkan/vulkan_core.h>

#include <filesystem>
#include <memory>
#include <utility>

namespace vv
{

PBRRenderSystem::PBRRenderSystem(
    std::shared_ptr<Device> device,
    VkRenderPass renderPass,
    const std::filesystem::path& vertexShaderPath,
    const std::filesystem::path& fragmentShaderPath,
    VkDescriptorSetLayout globalSetLayout
)
    : IRenderSystem(std::move(device))
    , m_materialSetLayout{ DescriptorSetLayout::Builder(this->device)
                               .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                               .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                               .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                               .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                               .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                               .build() }
{
    PBRRenderSystem::createGraphicsPipelineLayout(globalSetLayout);
    PBRRenderSystem::createGraphicsPipeline(renderPass, vertexShaderPath, fragmentShaderPath);
}

PBRRenderSystem::~PBRRenderSystem()
{
    vkDestroyPipelineLayout(device->device(), m_graphicsPipelineLayout, nullptr);
}

void PBRRenderSystem::render(const FrameInfo& frameInfo) const
{
    // TODO: Implement drawing functionality
}

void PBRRenderSystem::createGraphicsPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    // NOTE: These push constants provide the factors for the material system
    constexpr VkPushConstantRange pushConstantRangeMaterial{ .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                                                             .offset = 0,
                                                             .size = sizeof(MaterialPushConstants) };

    // NOTE: These push constants conatin the model and normal matrix
    constexpr VkPushConstantRange pushConstantRangeModel{ .stageFlags
                                                          = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                                          .offset = 0,
                                                          .size = sizeof(SimplePushConstantData) };

    std::vector<VkPushConstantRange> pushConstantRanges{ pushConstantRangeMaterial, pushConstantRangeModel };

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout,
                                                             m_materialSetLayout->getDescriptorLayout() };

    VkPipelineLayoutCreateInfo layoutCI{};
    layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCI.setLayoutCount = static_cast<std::uint32_t>(descriptorSetLayouts.size());
    layoutCI.pSetLayouts = descriptorSetLayouts.data();
    layoutCI.pushConstantRangeCount = static_cast<std::uint32_t>(pushConstantRanges.size());
    layoutCI.pPushConstantRanges = pushConstantRanges.data();

    const VkResult result{ vkCreatePipelineLayout(device->device(), &layoutCI, nullptr, &m_graphicsPipelineLayout) };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to create PBR Pipeline layout", result);
}

void PBRRenderSystem::createGraphicsPipeline(
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
