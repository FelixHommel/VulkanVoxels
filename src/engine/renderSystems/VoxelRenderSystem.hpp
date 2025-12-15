#ifndef VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_VOXEL_RENDER_SYSTEM_HPP
#define VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_VOXEL_RENDER_SYSTEM_HPP

#include "core/ComputePipeline.hpp"
#include "renderSystems/IRenderSystem.hpp"
#include "utility/FrameInfo.hpp"

namespace vv
{

/// \brief Render system that can render voxelized meshes
///
/// \author Felix Hommel
/// \date 12/13/2025
class VoxelRenderSystem final : public IRenderSystem
{
public:
    /// \brief Create a new \ref VoxelRenderSystem
    ///
    /// \param device the \ref Device used to create pipelines
    /// \param renderPass which render pass to use for the graphics pipeline
    /// \param computeShaderPath filepath to the compute shader
    /// \param vertexShaderPath filepath to the vertex shader
    /// \param fragmentShaderPath filepath to the fragment shader
    /// \param globalSetLayout layout of the global descriptor set
    explicit VoxelRenderSystem(
        std::shared_ptr<Device> device,
        VkRenderPass renderPass,
        const std::filesystem::path& computeShaderPath,
        const std::filesystem::path& vertexShaderPath,
        const std::filesystem::path& fragmentShaderPath,
        VkDescriptorSetLayout globalSetLayout
    );
    ~VoxelRenderSystem() override;

    VoxelRenderSystem(const VoxelRenderSystem&) = delete;
    VoxelRenderSystem(VoxelRenderSystem&&) = delete;
    VoxelRenderSystem& operator=(const VoxelRenderSystem&) = delete;
    VoxelRenderSystem& operator=(VoxelRenderSystem&&) = delete;

    /// \brief Render voxelized meshes
    ///
    /// \param frameInfo \ref FrameInfo with data about the current frame
    void render(const FrameInfo& frameInfo) const override;

private:
    VkPipelineLayout m_computePipelineLayout{ VK_NULL_HANDLE };
    std::unique_ptr<ComputePipeline> m_computePipeline;

    void createComputePipelineLayout();
    void createComputePipeline(const std::filesystem::path& computeShaderPath);
    void createGraphicsPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
    void createGraphicsPipeline(
        VkRenderPass renderPass,
        const std::filesystem::path& vertexShaderPath,
        const std::filesystem::path& fragmentShaderPath
    ) override;
};

} // namespace vv

#endif // ! VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_VOXEL_RENDER_SYSTEM_HPP
