#include "BasicRenderSystem.hpp"

#include "core/Device.hpp"
#include "core/Pipeline.hpp"
#include "utility/FrameInfo.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace vv
{

BasicRenderSystem::BasicRenderSystem(
	std::shared_ptr<Device> device,
	VkRenderPass renderPass,
	VkDescriptorSetLayout globalSetLayout
)
	: device{ std::move(device) }
{
	createPipelineLayout(globalSetLayout);
	createPipeline(renderPass);
}

BasicRenderSystem::~BasicRenderSystem()
{
	vkDestroyPipelineLayout(device->device(), m_pipelineLayout, nullptr);
}

void BasicRenderSystem::renderObjects(FrameInfo& frameInfo) const
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

	for(auto& [_, obj] : *frameInfo.objects)
	{
		if(obj.model == nullptr)
			continue;

		const SimplePushConstantData pushData{
			.modelMatrix = obj.transform.mat4(),
			.normalMatrix = obj.transform.normalMatrix()
		};

		vkCmdPushConstants(
			frameInfo.commandBuffer,
			m_pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(SimplePushConstantData),
			&pushData
		);

		obj.model->bind(frameInfo.commandBuffer);
		obj.model->draw(frameInfo.commandBuffer);
	}
}

/// \brief Create a PipelineLayout that can be used to create a Pipeline
void BasicRenderSystem::createPipelineLayout(
	VkDescriptorSetLayout globalSetLayout
)
{
	constexpr VkPushConstantRange pushConstantRange{
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.offset = 0,
		.size = sizeof(SimplePushConstantData)
	};
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.setLayoutCount =
		static_cast<std::uint32_t>(descriptorSetLayouts.size());
	createInfo.pSetLayouts = descriptorSetLayouts.data();
	createInfo.pushConstantRangeCount = 1;
	createInfo.pPushConstantRanges = &pushConstantRange;

	if(vkCreatePipelineLayout(
		   device->device(), &createInfo, nullptr, &m_pipelineLayout
	   ) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout");
}

/// \brief Create a Pipeline for Rendering
void BasicRenderSystem::createPipeline(VkRenderPass renderPass)
{
#if defined(VV_ENABLE_ASSERTS)
	assert(
		m_pipelineLayout != VK_NULL_HANDLE &&
		"Cannot create pipeline without pipeline layout"
	);
#endif

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = m_pipelineLayout;

	m_pipeline = std::make_unique<Pipeline>(
		device, VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH, pipelineConfig
	);
}

} // namespace vv
