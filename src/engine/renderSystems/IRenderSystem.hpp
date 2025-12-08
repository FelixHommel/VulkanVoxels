#ifndef VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_RENDER_SYSTEM_BASE_HPP
#define VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_RENDER_SYSTEM_BASE_HPP

#include "core/Device.hpp"
#include "core/Pipeline.hpp"
#include "utility/FrameInfo.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <utility>

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

/// \brief Base interface for render systems
///
/// \author Felix Hommel
/// \date 12/8/2025
class IRenderSystem
{
public:
    explicit IRenderSystem(std::shared_ptr<Device> device)
        : device(std::move(device))
    {
    }

    virtual ~IRenderSystem() = default;

    IRenderSystem(const IRenderSystem&) = delete;
    IRenderSystem(IRenderSystem&&) = delete;
    IRenderSystem& operator=(const IRenderSystem&) = delete;
    IRenderSystem& operator=(IRenderSystem&&) = delete;

    /// \brief Update whatever the render system has to update.
    /// \note This may do nothing for certain render systems that don't have to update anything
    ///
    /// \param frameInfo \ref FrameInfo important frame related data
    virtual void update(FrameInfo& frameInfo, GlobalUBO& ubo) {}
    /// \brief Render whatever the render system is responsible for
    ///
    /// \param frameInfo \ref FrameInfo with data about the current frame
    virtual void render(const FrameInfo& frameInfo) const = 0;

protected:
    std::shared_ptr<Device> device;

    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };

    virtual void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    virtual void createPipeline(VkRenderPass renderPass, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_RENDER_SYSTEMS_RENDER_SYSTEM_BASE_HPP
