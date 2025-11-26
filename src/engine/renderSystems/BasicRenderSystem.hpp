#ifndef VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_BASIC_RENDER_SYSTEM_HPP
#define VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_BASIC_RENDER_SYSTEM_HPP

#include "core/Device.hpp"
#include "core/Pipeline.hpp"
#include "utility/FrameInfo.hpp"
#include "utility/Object.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>

namespace vv
{

/// \brief Structure containing the information about push constants
///
/// Representation of which data is sent to the GPU via push constant
///
/// \author Felix Hommel
/// \date 11/19/2025
struct SimplePushConstantData
{
    glm::mat4 modelMatrix{ 1.f };
    glm::mat4 normalMatrix{ 1.f };
};

/// \brief Render System to render standard \ref Object
///
/// Used to render simple \ref Object by first pushing the push constants and then
/// drawing the mesh
///
/// \author Felix Hommel
/// \date 11/19/2025
class BasicRenderSystem
{
public:
    /// \brief Create a new \ref BasicRenderSystem
    ///
    /// \param device Reference to a \ref Device to create the \ref Pipeline on
    /// \param renderPass Which RenderPass to use in the pipeline
    /// \param gloablSetLayout the layout of globally used descriptor sets
    BasicRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~BasicRenderSystem();

    BasicRenderSystem(const BasicRenderSystem&) = delete;
    BasicRenderSystem(BasicRenderSystem&&) = delete;
    BasicRenderSystem& operator=(const BasicRenderSystem&) = delete;
    BasicRenderSystem& operator=(BasicRenderSystem&&) = delete;

    /// \brief Render a list of \ref Object
    ///
    /// \param frameInfo \ref FrameInfo with data about the current frame
    /// \param objects a vector of \ref Object which are the objects that are being rendered
    void renderObjects(FrameInfo& frameInfo, std::vector<Object>& objects) const;

private:
    static constexpr auto VERTEX_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/simpleVert.spv" };
    static constexpr auto FRAGMENT_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/simpleFrag.spv" };

    Device& device;

    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };

    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);
};

} // !vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_BASIC_RENDER_SYSTEM_HPP
