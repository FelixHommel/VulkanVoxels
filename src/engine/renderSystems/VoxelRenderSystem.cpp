#include "VoxelRenderSystem.hpp"

#include "renderSystems/IRenderSystem.hpp"

#include <memory>

namespace vv
{

VoxelRenderSystem::VoxelRenderSystem(std::shared_ptr<Device> device) : IRenderSystem(std::move(device))
{
    // TODO: Setup voxel renderer
}

VoxelRenderSystem::~VoxelRenderSystem()
{
    // TODO: Destroy voxel renderer
}

void VoxelRenderSystem::render(const FrameInfo& frameInfo) const
{
    // TODO: Implement drawing functionality
}

void VoxelRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    // TODO: Implement voxel pipeline layout
}

void VoxelRenderSystem::createPipeline(
    VkRenderPass renderPass,
    const std::filesystem::path& vertexShaderPath,
    const std::filesystem::path& fragmentShaderPath
)
{
    // TODO: Implement voxel pipeline (compute, graphics)
}

} // namespace vv
