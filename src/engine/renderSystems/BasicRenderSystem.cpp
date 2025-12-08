#include "BasicRenderSystem.hpp"

#include "core/Device.hpp"
#include "core/Pipeline.hpp"
#include "utility/FrameInfo.hpp"
#include "utility/object/components/ModelComponent.hpp"
#include "utility/object/components/TransformComponent.hpp"

#include <vulkan/vulkan_core.h>

#include <memory>
#include <ranges>
#include <utility>

namespace vv
{

BasicRenderSystem::BasicRenderSystem(
    std::shared_ptr<Device> device,
    VkRenderPass renderPass,
    VkDescriptorSetLayout globalSetLayout
)
    : IRenderSystem(std::move(device))
{
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass, VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
}

BasicRenderSystem::~BasicRenderSystem()
{
    vkDestroyPipelineLayout(device->device(), m_pipelineLayout, nullptr);
}

void BasicRenderSystem::render(const FrameInfo& frameInfo) const
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
        nullptr
    );

    for(auto& obj : *frameInfo.objects | std::views::values)
    {
        if(!obj.hasComponent<ModelComponent>())
            continue;

        const SimplePushConstantData pushData{ .modelMatrix = obj.getComponent<TransformComponent>()->mat4(),
                                               .normalMatrix = obj.getComponent<TransformComponent>()->normalMatrix() };

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            m_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &pushData
        );

        obj.getComponent<ModelComponent>()->model->bind(frameInfo.commandBuffer);
        obj.getComponent<ModelComponent>()->model->draw(frameInfo.commandBuffer);
    }
}

} // namespace vv
