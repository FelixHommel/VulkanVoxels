#include "BasicRenderSystem.hpp"

#include "Camera.hpp"
#include "Object.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "GLFW/glfw3.h"

#include <array>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vv
{

BasicRenderSystem::BasicRenderSystem(Device& device, VkRenderPass renderPass)
    : device(device)
{
    createPipelineLayout();
    createPipeline(renderPass);
}

BasicRenderSystem::~BasicRenderSystem()
{
    vkDestroyPipelineLayout(device.device(), m_pipelineLayout, nullptr);
}

void BasicRenderSystem::renderObjects(VkCommandBuffer commandBuffer, std::vector<Object>& objects, const Camera& camera)
{
    m_pipeline->bind(commandBuffer);

    for(auto& obj : objects)
    {
        obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
        obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.005f, glm::two_pi<float>());

        SimplePushConstantData pushData{
            .transform = camera.getProjection() * obj.transform.mat4(),
            .color = obj.color
        };

        vkCmdPushConstants(
            commandBuffer,
            m_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &pushData);

        obj.model->bind(commandBuffer);
        obj.model->draw(commandBuffer);
    }
}

void BasicRenderSystem::createPipelineLayout()
{
    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(SimplePushConstantData)
    };

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = 0;
    createInfo.pSetLayouts = nullptr;
    createInfo.pushConstantRangeCount = 1;
    createInfo.pPushConstantRanges = &pushConstantRange;

    if(vkCreatePipelineLayout(device.device(), &createInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout");
}

void BasicRenderSystem::createPipeline(VkRenderPass renderPass)
{
#if defined(VV_ENABLE_ASSERTS)
    assert(m_pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline without pipeline layout");
#endif

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;

    m_pipeline = std::make_unique<Pipeline>(device, VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH, pipelineConfig);
}

} // !vv
