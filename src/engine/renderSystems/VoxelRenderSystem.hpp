#ifndef VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_VOXEL_RENDER_SYSTEM_HPP
#define VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_VOXEL_RENDER_SYSTEM_HPP

#include "renderSystems/IRenderSystem.hpp"
#include "utility/FrameInfo.hpp"

namespace vv
{

// TODO: Add Doxygen comments
class VoxelRenderSystem final : public IRenderSystem
{
public:
    explicit VoxelRenderSystem(std::shared_ptr<Device> device);
    ~VoxelRenderSystem() override;

    VoxelRenderSystem(const VoxelRenderSystem&) = delete;
    VoxelRenderSystem(VoxelRenderSystem&&) = delete;
    VoxelRenderSystem& operator=(const VoxelRenderSystem&) = delete;
    VoxelRenderSystem& operator=(VoxelRenderSystem&&) = delete;

    void render(const FrameInfo& frameInfo) const override;

private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
    void createPipeline(
        VkRenderPass renderPass,
        const std::filesystem::path& vertexShaderPath,
        const std::filesystem::path& fragmentShaderPath
    ) override;
};

} // namespace vv

#endif // ! VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_VOXEL_RENDER_SYSTEM_HPP
