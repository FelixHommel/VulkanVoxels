#include "GraphicsPipeline.hpp"

#include "core/Device.hpp"
#include "core/IPipeline.hpp"
#include "utility/Model.hpp"
#include "utility/exceptions/VulkanException.hpp"

#include <vulkan/vulkan_core.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

namespace vv
{

GraphicsPipeline::GraphicsPipeline(
    std::shared_ptr<Device> device,
    const std::filesystem::path& vertexShaderPath,
    const std::filesystem::path& fragmentShaderPath,
    const GraphicsPipelineConfigInfo& configInfo
)
    : IPipeline(std::move(device))
{
#if defined(VV_ENABLE_ASSERTS)
    assert(
        configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipeline layout provided"
    );
    assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no render pass provided");
#endif

    const auto vertCode{ readFile(vertexShaderPath) };
    const auto fragCode{ readFile(fragmentShaderPath) };

    createShaderModule(vertCode, &m_vertexShaderModule);
    createShaderModule(fragCode, &m_fragmentShaderModule);

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = m_vertexShaderModule;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = m_fragmentShaderModule;
    shaderStages[1].pName = "main";

    const auto& bindingDescriptions{ configInfo.bindingDescription };
    const auto& attributeDescriptions{ configInfo.attributeDescription };
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<std::uint32_t>(bindingDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pViewportState = &configInfo.viewportInfo;
    pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
    pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
    pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
    pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
    pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;
    pipelineInfo.layout = configInfo.pipelineLayout;
    pipelineInfo.renderPass = configInfo.renderPass;
    pipelineInfo.subpass = configInfo.subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    const VkResult result{
        vkCreateGraphicsPipelines(m_device->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline)
    };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to create graphics pipeline", result);
}

GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyShaderModule(m_device->device(), m_vertexShaderModule, nullptr);
    vkDestroyShaderModule(m_device->device(), m_fragmentShaderModule, nullptr);
    vkDestroyPipeline(m_device->device(), m_pipeline, nullptr);
}

void GraphicsPipeline::bind(VkCommandBuffer commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
}

void GraphicsPipeline::defaultGraphicsPipelineConfigInfo(GraphicsPipelineConfigInfo& configInfo)
{
    configInfo.bindingDescription = Model::Vertex::getBindingDescriptions();
    configInfo.attributeDescription = Model::Vertex::getAttributeDescriptions();

    configInfo.viewportInfo = {};
    configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    configInfo.viewportInfo.viewportCount = 1;
    configInfo.viewportInfo.pViewports = nullptr;
    configInfo.viewportInfo.scissorCount = 1;
    configInfo.viewportInfo.pScissors = nullptr;

    configInfo.inputAssemblyInfo = {};
    configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    configInfo.rasterizationInfo = {};
    configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
    configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
    configInfo.rasterizationInfo.depthBiasConstantFactor = 0.f;
    configInfo.rasterizationInfo.depthBiasClamp = 0.f;
    configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.f;
    configInfo.rasterizationInfo.lineWidth = 1.f;

    configInfo.multisampleInfo = {};
    configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
    configInfo.multisampleInfo.minSampleShading = 1.f;
    configInfo.multisampleInfo.pSampleMask = nullptr;
    configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

    configInfo.colorBlendAttachment = {};
    configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
    configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    configInfo.colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    configInfo.colorBlendInfo = {};
    configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
    configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
    configInfo.colorBlendInfo.attachmentCount = 1;
    configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
    configInfo.colorBlendInfo.blendConstants[0] = 0.f;
    configInfo.colorBlendInfo.blendConstants[0] = 0.f;
    configInfo.colorBlendInfo.blendConstants[0] = 0.f;
    configInfo.colorBlendInfo.blendConstants[0] = 0.f;

    configInfo.depthStencilInfo = {};
    configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.front = {};
    configInfo.depthStencilInfo.back = {};
    configInfo.depthStencilInfo.minDepthBounds = 0.f;
    configInfo.depthStencilInfo.maxDepthBounds = 1.f;

    configInfo.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    configInfo.dynamicStateInfo = {};
    configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    configInfo.dynamicStateInfo.dynamicStateCount = static_cast<std::uint32_t>(configInfo.dynamicStateEnables.size());
    configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
}

} // namespace vv
