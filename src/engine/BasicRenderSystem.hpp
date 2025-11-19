#ifndef SRC_ENGINE_BASIC_RENDER_SYSTEM_HPP
#define SRC_ENGINE_BASIC_RENDER_SYSTEM_HPP

#include "Camera.hpp"
#include "Object.hpp"
#include "Device.hpp"
#include "Pipeline.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include <vulkan/vulkan_core.h>

#include <memory>

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
    glm::mat4 transform{ 1.f };
    alignas(16) glm::vec3 color{};
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
    BasicRenderSystem(Device& device, VkRenderPass renderPass);
    ~BasicRenderSystem();

    BasicRenderSystem(const BasicRenderSystem&) = delete;
    BasicRenderSystem(BasicRenderSystem&&) = delete;
    BasicRenderSystem& operator=(const BasicRenderSystem&) = delete;
    BasicRenderSystem& operator=(BasicRenderSystem&&) = delete;

    /// \brief Render a list of \ref Object
    ///
    /// \param commandBuffer which VkCommandBuffer is drawn to
    /// \param objects a vector of \ref Object which are the objcts that are being rendered
    /// \param camera the \ref Camera which is in use
    void renderObjects(VkCommandBuffer commandBuffer, std::vector<Object>& objects, const Camera& camera);

private:
    static constexpr auto VERTEX_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/simpleVert.spv" };
    static constexpr auto FRAGMENT_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/simpleFrag.spv" };

    Device& device;

    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };

    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);
};

} // !vv

#endif // !SRC_ENGINE_BASIC_RENDER_SYSTEM_HPP
