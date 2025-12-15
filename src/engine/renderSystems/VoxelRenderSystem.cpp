#include "VoxelRenderSystem.hpp"

#include "renderSystems/IRenderSystem.hpp"

#include <memory>

namespace vv
{

VoxelRenderSystem::VoxelRenderSystem(
    std::shared_ptr<Device> device,
    VkRenderPass renderPass,
    const std::filesystem::path& computeShaderPath,
    const std::filesystem::path& vertexShaderPath,
    const std::filesystem::path& fragmentShaderPath,
    VkDescriptorSetLayout globalSetLayout
)
    : IRenderSystem(std::move(device))
{
    // TODO: Setup voxel renderer
    createComputePipelineLayout();
    createComputePipeline(computeShaderPath);
    VoxelRenderSystem::createGraphicsPipelineLayout(globalSetLayout);
    VoxelRenderSystem::createGraphicsPipeline(renderPass, vertexShaderPath, fragmentShaderPath);
}

VoxelRenderSystem::~VoxelRenderSystem()
{
    vkDestroyPipelineLayout(device->device(), m_graphicsPipelineLayout, nullptr);
    vkDestroyPipelineLayout(device->device(), m_computePipelineLayout, nullptr);
}

void VoxelRenderSystem::render(const FrameInfo& frameInfo) const
{
    // TODO: Implement drawing functionality
}

void VoxelRenderSystem::createComputePipelineLayout()
{
    // NOTE: Planned voxelization shader descriptors
    // Set 0: Storage buffers (geometry in)
    //  Binding 0: Vertex Buffer (SSBO - read only)
    //  Binding 1: Index Buffer (SSBO - read only)
    // Set 1: Storage images (voxel data output)
    //  Binding 0: Albedo (3D image - write only)
    //  Binding 1: (Future work) emission (3D image - write only)
    //  Binding 2: (Future work) normals (3D image - write only)
}

void VoxelRenderSystem::createComputePipeline(const std::filesystem::path& computeShaderPath) {}

void VoxelRenderSystem::createGraphicsPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    // TODO: Implement voxel pipeline layout
}

void VoxelRenderSystem::createGraphicsPipeline(
    VkRenderPass renderPass,
    const std::filesystem::path& vertexShaderPath,
    const std::filesystem::path& fragmentShaderPath
)
{
    // TODO: Implement voxel pipeline (compute, graphics)
}

} // namespace vv
