#ifndef VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_BASIC_RENDER_SYSTEM_HPP
#define VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_BASIC_RENDER_SYSTEM_HPP

#include "core/Device.hpp"
#include "renderSystems/IRenderSystem.hpp"
#include "utility/FrameInfo.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include <vulkan/vulkan_core.h>

#include <memory>

namespace vv
{

/// \brief Render System to render standard \ref Object
///
/// Used to render simple \ref Object by first pushing the push constants and then
/// drawing the mesh
///
/// \author Felix Hommel
/// \date 11/19/2025
class BasicRenderSystem final : public IRenderSystem
{
public:
    /// \brief Create a new \ref BasicRenderSystem
    ///
    /// \param device \ref Device to create the \ref Pipeline on
    /// \param renderPass Which RenderPass to use in the pipeline
    /// \param globalSetLayout the layout of globally used descriptor sets
    BasicRenderSystem(std::shared_ptr<Device> device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~BasicRenderSystem() override;

    BasicRenderSystem(const BasicRenderSystem&) = delete;
    BasicRenderSystem(BasicRenderSystem&&) = delete;
    BasicRenderSystem& operator=(const BasicRenderSystem&) = delete;
    BasicRenderSystem& operator=(BasicRenderSystem&&) = delete;

    /// \brief Render a list of \ref Object
    ///
    /// \param frameInfo \ref FrameInfo with data about the current frame
    void render(const FrameInfo& frameInfo) const override;

private:
    static constexpr auto VERTEX_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/simpleVert.spv" };
    static constexpr auto FRAGMENT_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/simpleFrag.spv" };
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_BASIC_RENDER_SYSTEM_HPP
