#ifndef VULKAN_VOXELS_SRC_ENGINE_CORE_PIPELINE_HPP
#define VULKAN_VOXELS_SRC_ENGINE_CORE_PIPELINE_HPP

#include "Device.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>
#include <filesystem>

namespace vv
{

/// \brief Save the configuration state of all pipeline components
///
/// \author Felix Hommel
/// \date 11//13/2025
struct PipelineConfigInfo
{
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
class Pipeline
{
public:
    /// \brief Create a new \ref Pipeline
    ///
    /// \param device \ref Device where the pipeline is created on
    /// \param vertexShaderPath filepath to the vertex shader in spir-v format
    /// \param fragmentShaderPath filepath to the fragment shader in spir-v format
    /// \param configInfo \ref PipelineConfigInfo that contains information of how the pipeline should be configured
    Pipeline(Device& device,
            const std::filesystem::path& vertexShaderPath,
            const std::filesystem::path& fragmentShaderPath,
            const PipelineConfigInfo& configInfo);
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline(Pipeline&&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;

    /// \brief Provides a default set of configuration that can be used for configuration
    ///
    /// \param configInfo \ref PipelineConfigInfo where the configuration is being saved
    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

    /// \brief Bind the Pipeline to a command buffer
    ///
    /// \param commandBuffer the VkCommandBuffer to which the pipeline is being bound
    void bind(VkCommandBuffer commandBuffer) const;

private:
    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) const;

    static std::vector<char> readFile(const std::filesystem::path& filepath);

    Device& device;
    VkPipeline m_graphicsPipeline{};
    VkShaderModule m_vertexShaderModule{};
    VkShaderModule m_fragmentShaderModule{};
};

}; // !vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_CORE_PIPELINE_HPP
