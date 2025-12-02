#ifndef VULKAN_VOXELS_SRC_ENGINE_CORE_SWAPCHAIN_HPP
#define VULKAN_VOXELS_SRC_ENGINE_CORE_SWAPCHAIN_HPP

#include "Device.hpp"

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace vv
{

/// \brief Manages synchronization of command buffers on a low level
///
/// \author Felix Hommel
/// \date 11/19/2025
class Swapchain
{
public:
	/// \brief Create a new \ref Swapchain
	///
	/// \param device \ref Device that is used to create the Swapchain
	/// \param windowExtent size of the window used to set the size for frame buffers
	Swapchain(std::shared_ptr<Device> device, VkExtent2D windowExtent);
	/// \brief Create a new \ref Swapchain
	///
	/// This variant is used primarily for recreation of the swapchain therefore
	/// it can be passed the old swapchain which can boost creation speed
	///
	/// \param device \ref Device that is used to create the Swapchain
	/// \param windowExtent size of the window to set the size for frame buffers
	/// \param previous the olf \ref Swapchain wrapped in a shared_ptr
	Swapchain(
		std::shared_ptr<Device> device,
		VkExtent2D windowExtent,
		std::shared_ptr<Swapchain> previous
	);
	~Swapchain();

	Swapchain(const Swapchain&) = delete;
	Swapchain(Swapchain&&) = delete;
	Swapchain& operator=(const Swapchain&) = delete;
	Swapchain& operator=(Swapchain&&) = delete;

	static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT{ 2 };

	[[nodiscard]] std::size_t imageCount() const
	{
		return m_swapchainImages.size();
	}
	[[nodiscard]] VkExtent2D getExtent() const
	{
		return m_swapchainImageExtent;
	}
	[[nodiscard]] float extentAspectRatio() const noexcept
	{
		return static_cast<float>(m_swapchainImageExtent.width) /
		       static_cast<float>(m_swapchainImageExtent.height);
	}

	/** Presentation utility */
	[[nodiscard]] VkResult acquireNextImage(std::uint32_t* imageIndex) const;
	[[nodiscard]] VkResult submitCommandBuffer(
		const VkCommandBuffer* commandBuffer,
		const std::uint32_t* imageIndex
	);

	/** Raw handle access */
	[[nodiscard]] VkSwapchainKHR getHandle() const noexcept
	{
		return m_swapchain;
	}
	[[nodiscard]] VkRenderPass getRenderPass() const noexcept
	{
		return m_renderPass;
	}
	[[nodiscard]] VkFramebuffer getFramebuffer(std::size_t index) const;

	/// \brief Check if the swapchain formats match
	///
	/// Compare the image and depth format to check if they are compatible
	///
	/// \param swapchain the swapchain to compare formats with
	[[nodiscard]] bool compareSwapFormats(
		const Swapchain& swapchain
	) const noexcept;

private:
	/** External objects */
	std::shared_ptr<Device> device;
	VkExtent2D windowExtent;

	/** Core */
	VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };
	std::shared_ptr<Swapchain> m_oldSwapchain{ nullptr };
	std::vector<VkFramebuffer> m_swapchainFramebuffers{ VK_NULL_HANDLE };
	VkRenderPass m_renderPass{ VK_NULL_HANDLE };

	/** Images */
	VkFormat m_swapchainImageFormat{};
	VkFormat m_swapchainDepthFormat{};
	VkExtent2D m_swapchainImageExtent{};
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	std::vector<VkImage> m_depthImages;
	std::vector<VkDeviceMemory> m_depthImagesMemory;
	std::vector<VkImageView> m_depthImageViews;
	std::size_t m_currentFrame{ 0 };

	/** Sync */
	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
	std::vector<VkFence> m_imagesInFlight;

	/** Setup functions */
	void createSwapchain();
	void createImageViews();
	void createRenderPass();
	void createDepthResources();
	void createFramebuffers();
	void createSyncObjects();

	/** Supporting methods */
	[[nodiscard]] VkExtent2D chooseSwapExtent(
		const VkSurfaceCapabilitiesKHR& capabilities
	) const;
	[[nodiscard]] VkFormat findDepthFormat() const;

	/** Supporting functions */
	static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& availableFormats
	);
	static VkPresentModeKHR chooseSwapPresentMode(
		const std::vector<VkPresentModeKHR>& availablePresentModes
	);
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_CORE_SWAPCHAIN_HPP
