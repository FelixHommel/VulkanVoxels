#ifndef VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_PBR_RENDER_SYSTEM_HPP
#define VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_PBR_RENDER_SYSTEM_HPP

#include "core/DescriptorSetLayout.hpp"
#include "core/Device.hpp"
#include "renderSystems/IRenderSystem.hpp"
#include "utility/FrameInfo.hpp"

#include <vulkan/vulkan_core.h>

#include <filesystem>
#include <memory>

namespace vv
{

/// \brief Render system that can render voxelized meshes
///
/// \author Felix Hommel
/// \date 12/13/2025
class PBRRenderSystem final : public IRenderSystem
{
public:
    /// \brief Create a new \ref VoxelRenderSystem
    ///
    /// \param device the \ref Device used to create pipelines
    /// \param renderPass which render pass to use for the graphics pipeline
    /// \param vertexShaderPath filepath to the vertex shader
    /// \param fragmentShaderPath filepath to the fragment shader
    /// \param globalSetLayout layout of the global descriptor set
    explicit PBRRenderSystem(
        std::shared_ptr<Device> device,
        VkRenderPass renderPass,
        const std::filesystem::path& vertexShaderPath,
        const std::filesystem::path& fragmentShaderPath,
        VkDescriptorSetLayout globalSetLayout
    );
    ~PBRRenderSystem() override;

    PBRRenderSystem(const PBRRenderSystem&) = delete;
    PBRRenderSystem(PBRRenderSystem&&) = delete;
    PBRRenderSystem& operator=(const PBRRenderSystem&) = delete;
    PBRRenderSystem& operator=(PBRRenderSystem&&) = delete;

    /// \brief Render voxelized meshes
    ///
    /// \param frameInfo \ref FrameInfo with data about the current frame
    void render(const FrameInfo& frameInfo) const override;

private:
    std::unique_ptr<DescriptorSetLayout> m_materialSetLayout;

    void createGraphicsPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
    void createGraphicsPipeline(
        VkRenderPass renderPass,
        const std::filesystem::path& vertexShaderPath,
        const std::filesystem::path& fragmentShaderPath
    ) override;
};

} // namespace vv

#endif // ! VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_PBR_RENDER_SYSTEM_HPP
