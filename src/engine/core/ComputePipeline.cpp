#include "ComputePipeline.hpp"

#include "core/Device.hpp"
#include "core/IPipeline.hpp"
#include "utility/exceptions/VulkanException.hpp"

#include <vulkan/vulkan_core.h>

#include <filesystem>
#include <memory>
#include <utility>

namespace vv
{

ComputePipeline::ComputePipeline(
    std::shared_ptr<Device> device,
    const std::filesystem::path& computeShaderPath,
    const VkPipelineLayout& pipelineLayout
)
    : IPipeline(std::move(device))
{
    const auto computeCode{ readFile(computeShaderPath) };
    createShaderModule(computeCode, &m_computeShaderModule);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = m_computeShaderModule;
    computeShaderStageInfo.pName = "main";

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = computeShaderStageInfo;
    pipelineInfo.layout = pipelineLayout;

    const VkResult result{
        vkCreateComputePipelines(m_device->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline)
    };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to create compute pipeline", result);
}

ComputePipeline::~ComputePipeline()
{
    vkDestroyShaderModule(m_device->device(), m_computeShaderModule, nullptr);
    vkDestroyPipeline(m_device->device(), m_pipeline, nullptr);
}

void ComputePipeline::bind(VkCommandBuffer commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
}

} // namespace vv
