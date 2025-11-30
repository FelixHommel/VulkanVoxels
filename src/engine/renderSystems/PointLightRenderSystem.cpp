#include "PointLightRenderSystem.hpp"

#include "core/Device.hpp"
#include "core/Pipeline.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "utility/FrameInfo.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
//#include "glm/gtc/constants.hpp"
//#include "glm/gtc/matrix_transform.hpp"
#include <vulkan/vulkan_core.h>

#include <cassert>
#include <memory>
#include <stdexcept>
#include <vector>

namespace vv
{

PointLightRenderSystem::PointLightRenderSystem(std::shared_ptr<Device> device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    : device{ std::move(device) }
{
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

PointLightRenderSystem::~PointLightRenderSystem()
{
    vkDestroyPipelineLayout(device->device(), m_pipelineLayout, nullptr);
}

void PointLightRenderSystem::update(FrameInfo& frameInfo, GlobalUBO& ubo) const
{
    auto rotateLight{ glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.dt, {0.f, -1.f, 0.f}) };
    int lightIndex{ 0 };
    for(auto& [_, obj] : frameInfo.objects)
    {
        if(obj.pointLight == nullptr)
            continue;

#if defined(VV_ENABLE_ASSERTS)
        assert(lightIndex < MAX_LIGHTS && "Point lights exceed the allowed maximum");
#endif

        obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));
        
        ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
        ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);

        lightIndex += 1;
    }
    ubo.numLights = lightIndex;
}

void PointLightRenderSystem::render(FrameInfo& frameInfo) const
{
    m_pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr);


    constexpr std::uint32_t squareVertexCount{ 6 };
    for(auto& [_, obj] : frameInfo.objects)
    {
        if(obj.pointLight == nullptr)
            continue;

        PointLightPushConstants push{
            .position = glm::vec4(obj.transform.translation, 1.f),
            .color = glm::vec4(obj.color, obj.pointLight->lightIntensity),
            .radius = obj.transform.scale.x
        };

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            m_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PointLightPushConstants),
            &push);

        vkCmdDraw(frameInfo.commandBuffer, squareVertexCount, 1, 0, 0);
    }
}

/// \brief Create a PipelineLayout that can be used to create a Pipeline
void PointLightRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(PointLightPushConstants)
    };
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = static_cast<std::uint32_t>(descriptorSetLayouts.size());
    createInfo.pSetLayouts = descriptorSetLayouts.data();
    createInfo.pushConstantRangeCount = 1;
    createInfo.pPushConstantRanges = &pushConstantRange;

    if(vkCreatePipelineLayout(device->device(), &createInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout");
}

/// \brief Create a Pipeline for Rendering
void PointLightRenderSystem::createPipeline(VkRenderPass renderPass)
{
#if defined(VV_ENABLE_ASSERTS)
    assert(m_pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline without pipeline layout");
#endif

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.bindingDescription.clear();
    pipelineConfig.attributeDescription.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;

    m_pipeline = std::make_unique<Pipeline>(device, VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH, pipelineConfig);
}

} // !vv
