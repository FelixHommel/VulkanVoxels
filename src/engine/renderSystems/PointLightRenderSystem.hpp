#ifndef VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_POINT_LIGHT_RENDER_SYSTEM_HPP
#define VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_POINT_LIGHT_RENDER_SYSTEM_HPP

#include "core/Device.hpp"
#include "renderSystems/IRenderSystem.hpp"
#include "utility/FrameInfo.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <string>

namespace vv
{

/// \brief Push constant definition for point lights
///
/// \author Felix Hommel
/// \date 11/28/2025
struct PointLightPushConstants
{
    glm::vec4 position{};
    glm::vec4 color{};
    float radius;
};

/// \brief Render System to render point lights in a billboard style
///
/// \author Felix Hommel
/// \date 11/27/2025
class PointLightRenderSystem final : public IRenderSystem
{
public:
    /// \brief Create a new \ref PointLightRenderSystem
    ///
    /// \param device \ref Device to create the \ref Pipeline on
    /// \param renderPass Which RenderPass to use in the pipeline
    /// \param globalSetLayout the layout of globally used descriptor sets
    PointLightRenderSystem(
        std::shared_ptr<Device> device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout
    );
    ~PointLightRenderSystem() override;

    PointLightRenderSystem(const PointLightRenderSystem&) = delete;
    PointLightRenderSystem(PointLightRenderSystem&&) = delete;
    PointLightRenderSystem& operator=(const PointLightRenderSystem&) = delete;
    PointLightRenderSystem& operator=(PointLightRenderSystem&&) = delete;

    /// \brief Update the point lights
    ///
    /// \param frameInfo \ref FrameInfo important frame related data
    void update(FrameInfo& frameInfo, GlobalUBO& ubo) override;

    /// \brief Render a point light
    ///
    /// \param frameInfo \ref FrameInfo with data about the current frame
    void render(const FrameInfo& frameInfo) const override;

private:
    static constexpr auto VERTEX_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/pointLightVert.spv" };
    static constexpr auto FRAGMENT_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/pointLightFrag.spv" };
    static constexpr std::uint32_t squareVertexCount{ 6 };

    void createGraphicsPipelineLayout(VkDescriptorSetLayout globalSetLayout) override;
    void createGraphicsPipeline(
        VkRenderPass renderPass,
        const std::filesystem::path& vertexShaderPath,
        const std::filesystem::path& fragmentShaderPath
    ) override;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_POINT_LIGHT_RENDER_SYSTEM_HPP
