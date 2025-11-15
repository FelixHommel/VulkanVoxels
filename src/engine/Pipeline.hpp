#ifndef SRC_ENGINE_PIPELINE_HPP
#define SRC_ENGINE_PIPELINE_HPP

#include "Device.hpp"

#include <vector>
#include <vulkan/vulkan_core.h>

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

/// \brief The Pipeline is a building piece that consits of programable and non-programable stages that vertices run through before they are displayed
///
/// \author Felix Hommel
/// \date 11/10/2025
class Pipeline
{
public:
    Pipeline(Device& device,
            const std::filesystem::path& vertexShaderPath,
            const std::filesystem::path& fragmentShaderPath,
            const PipelineConfigInfo& configInfo);
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline(Pipeline&&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;

    void bind(VkCommandBuffer commandBuffer);
    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

private:
    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

    Device& device;
    VkPipeline m_graphicsPipeline{};
    VkShaderModule m_vertexShaderModule{};
    VkShaderModule m_fragmentShaderModule{};
};

}; // !vv

#endif // !SRC_ENGINE_PIPELINE_HPP
