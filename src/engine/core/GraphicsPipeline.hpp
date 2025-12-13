#ifndef VULKAN_VOXELS_SRC_ENGINE_CORE_GRAPHICS_PIPELINE_HPP
#define VULKAN_VOXELS_SRC_ENGINE_CORE_GRAPHICS_PIPELINE_HPP

#include "core/Device.hpp"
#include "core/IPipeline.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

namespace vv
{

/// \brief Save the configuration state of all pipeline components
///
/// \author Felix Hommel
/// \date 11//13/2025
struct GraphicsPipelineConfigInfo
{
    std::vector<VkVertexInputBindingDescription> bindingDescription;
    std::vector<VkVertexInputAttributeDescription> attributeDescription;

    VkPipelineViewportStateCreateInfo viewportInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
    VkRenderPass renderPass{ VK_NULL_HANDLE };
    std::uint32_t subpass{ 0 };
};

/// \brief The Pipeline is a building piece that consists of programmable and non-programmable stages that vertices run through before they are displayed
///
/// \author Felix Hommel
/// \date 11/10/2025
class GraphicsPipeline final : public IPipeline
{
public:
    /// \brief Create a new \ref GraphicsPipeline
    ///
    /// \param device \ref Device where the pipeline is created on
    /// \param vertexShaderPath filepath to the vertex shader in spir-v format
    /// \param fragmentShaderPath filepath to the fragment shader in spir-v format
    /// \param configInfo \ref GraphicsPipelineConfigInfo that contains information of how the pipeline should be configured
    GraphicsPipeline(
        std::shared_ptr<Device> device,
        const std::filesystem::path& vertexShaderPath,
        const std::filesystem::path& fragmentShaderPath,
        const GraphicsPipelineConfigInfo& configInfo
    );
    ~GraphicsPipeline() override;

    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline(GraphicsPipeline&&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;

    /// \brief Provides a default set of configuration that can be used for configuration
    ///
    /// \param configInfo \ref GraphicsPipelineConfigInfo where the configuration is being saved
    static void defaultGraphicsPipelineConfigInfo(GraphicsPipelineConfigInfo& configInfo);

    void bind(VkCommandBuffer commandBuffer) const override;

private:
    VkShaderModule m_vertexShaderModule{};
    VkShaderModule m_fragmentShaderModule{};
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_CORE_GRAPHICS_PIPELINE_HPP
