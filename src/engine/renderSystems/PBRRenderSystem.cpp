#include "PBRRenderSystem.hpp"

#include "core/Device.hpp"
#include "renderSystems/IRenderSystem.hpp"
#include "utility/FrameInfo.hpp"

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
    // TODO: Implement PBR pipeline layout
}

void PBRRenderSystem::createGraphicsPipeline(
    VkRenderPass renderPass,
    const std::filesystem::path& vertexShaderPath,
    const std::filesystem::path& fragmentShaderPath
)
{
    // TODO: Implement PBR pipeline
}

} // namespace vv
