#include "Renderer.hpp"

#include "core/Device.hpp"
#include "core/Swapchain.hpp"
#include "core/Window.hpp"
#include "utility/exceptions/VulkanException.hpp"

#include "GLFW/glfw3.h"
#include <vulkan/vulkan_core.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace vv
{

Renderer::Renderer(
	std::shared_ptr<Window> window,
	std::shared_ptr<Device> device
)
	: window{ std::move(window) }
	, device{ std::move(device) }
{
	recreateSwapchain();
	createCommandBuffers();
}

Renderer::~Renderer()
{
	freeCommandBuffers();
}

VkCommandBuffer Renderer::getCurrentCommandBuffer() const
{
#if defined(VV_ENABLE_ASSERTS)
	assert(
		m_isFrameStarted &&
		"Cannot get command buffer when no frame is in progress"
	);
#endif

	return m_commandBuffers[m_currentFrameIndex];
}

std::size_t Renderer::getFrameIndex() const
{
#if defined(VV_ENABLE_ASSERTS)
	assert(
		m_isFrameStarted &&
		"Cannot get frame index when no frame is in progress"
	);
#endif

	return m_currentFrameIndex;
}

VkCommandBuffer Renderer::beginFrame()
{
#if defined(VV_ENABLE_ASSERTS)
	assert(
		!m_isFrameStarted &&
		"Cannot call beginFrame() while a frame is already in progress"
	);
#endif

	VkResult result{ m_swapchain->acquireNextImage(&m_currentImageIndex) };
	if(result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain();
		return nullptr;
	}

	if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw VulkanException("Failed to acquire swapchain image", result);

	m_isFrameStarted = true;

	auto* const commandBuffer{ getCurrentCommandBuffer() };
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	if(result != VK_SUCCESS)
		throw VulkanException(
			"Failed to begin recording command buffer", result
		);

	return commandBuffer;
}

void Renderer::endFrame()
{
#if defined(VV_ENABLE_ASSERTS)
	assert(
		m_isFrameStarted &&
		"Cannot call endFrame() while there is no frame in progress"
	);
#endif

	auto* const commandBuffer{ getCurrentCommandBuffer() };

	VkResult result{ vkEndCommandBuffer(commandBuffer) };
	if(result != VK_SUCCESS)
	{
		throw VulkanException("Failed to record command buffer", result);
	}

	result =
		m_swapchain->submitCommandBuffer(&commandBuffer, &m_currentImageIndex);
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
	   window->wasWindowResized())
	{
		window->resetWindowResizeFlag();
		recreateSwapchain();
	}
	else if(result != VK_SUCCESS)
		throw VulkanException("Failed to present swapchain image", result);

	m_isFrameStarted = false;
	m_currentFrameIndex =
		(m_currentFrameIndex + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginRenderPass(VkCommandBuffer commandBuffer) const
{
#if defined(VV_ENABLE_ASSERTS)
	assert(
		m_isFrameStarted &&
		"Cannot call beginRenderPass() while there is no frame in progress"
	);
	assert(
		commandBuffer == getCurrentCommandBuffer() &&
		"Cannot begin render pass on command buffer from a different frame"
	);
#endif

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { CLEAR_COLOR };
	clearValues[1].depthStencil = { .depth = 1.f, .stencil = 0 };

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_swapchain->getRenderPass();
	renderPassBeginInfo.framebuffer =
		m_swapchain->getFramebuffer(m_currentImageIndex);
	renderPassBeginInfo.renderArea = { .offset = { .x = 0, .y = 0 },
		                               .extent = m_swapchain->getExtent() };
	renderPassBeginInfo.clearValueCount =
		static_cast<std::uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(
		commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE
	);

	const VkViewport viewport{
		.x = 0.f,
		.y = 0.f,
		.width = static_cast<float>(m_swapchain->getExtent().width),
		.height = static_cast<float>(m_swapchain->getExtent().height),
		.minDepth = 0.f,
		.maxDepth = 1.f
	};

	const VkRect2D scissor{ .offset = { .x = 0, .y = 0 },
		                    .extent = m_swapchain->getExtent() };

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endRenderPass(VkCommandBuffer commandBuffer) const
{
#if defined(VV_ENABLE_ASSERTS)
	assert(
		m_isFrameStarted &&
		"Cannot call endRenderPass() while there is no frame in progress"
	);
	assert(
		commandBuffer == getCurrentCommandBuffer() &&
		"cannot end render pass on command buffer from a different frame"
	);
#endif

	vkCmdEndRenderPass(commandBuffer);
}

/// \brief Create as many command buffers as the amount of images that can be parallely in flight (supported by the swapchain)
void Renderer::createCommandBuffers()
{
	m_commandBuffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = device->commandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount =
		static_cast<std::uint32_t>(m_commandBuffers.size());

	const VkResult result{ vkAllocateCommandBuffers(
		device->device(), &allocInfo, m_commandBuffers.data()
	) };
	if(result != VK_SUCCESS)
		throw VulkanException("Failed to allocate command buffers", result);
}

/// \brief Free the allocated command buffers
void Renderer::freeCommandBuffers()
{
	vkFreeCommandBuffers(
		device->device(),
		device->commandPool(),
		static_cast<std::uint32_t>(m_commandBuffers.size()),
		m_commandBuffers.data()
	);
	m_commandBuffers.clear();
}

/// \brief Recreate the swapchain
///
/// The need for resizing the swapchain arises if the parameters have changed, especially
/// the size of the frame buffer
void Renderer::recreateSwapchain()
{
	auto extent{ window->getExtent() };
	while(extent.width == 0 || extent.height == 0)
	{
		extent = window->getExtent();
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(device->device());

	if(m_swapchain == nullptr)
		m_swapchain = std::make_unique<Swapchain>(device, extent);
	else
	{
		std::shared_ptr<Swapchain> oldSwapchain{ std::move(m_swapchain) };
		m_swapchain = std::make_unique<Swapchain>(device, extent, oldSwapchain);

		if(!oldSwapchain->compareSwapFormats(*m_swapchain))
			throw VulkanException(
				"Swapchain image or depth format has changed",
				VK_ERROR_OUT_OF_DATE_KHR
			);
	}
}

} // namespace vv
