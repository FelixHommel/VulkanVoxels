#ifndef SRC_ENGINE_SWAPCHAIN_HPP
#define SRC_ENGINE_SWAPCHAIN_HPP

#include "Device.hpp"

#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <cstdint>

namespace vv
{

class Swapchain
{
public:
    static constexpr auto MAX_FRAMES_IN_FLIGHT{ 2 };

    Swapchain(Device& device, VkExtent2D windowExtent);
    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    Swapchain(Swapchain&&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;
    Swapchain& operator=(Swapchain&&) = delete;

    [[nodiscard]] VkFramebuffer getFramebuffer(std::size_t index) const { return m_swapchainFramebuffers.at(index); }
    [[nodiscard]] VkRenderPass getRenderPass() const noexcept { return m_renderPass; }
    [[nodiscard]] VkImageView getImageView(std::size_t index) const { return m_swapchainImageViews.at(index); }
    [[nodiscard]] std::size_t imageCount() const noexcept { return m_swapchainImages.size(); }
    [[nodiscard]] VkFormat getSwapchainImageFormat() const noexcept { return m_swapchainImageFormat; }
    [[nodiscard]] VkExtent2D getSwapchainExtent() const noexcept { return m_swapchainExtent; }
    [[nodiscard]] std::uint32_t width() const noexcept { return m_swapchainExtent.width; }
    [[nodiscard]] std::uint32_t height() const noexcept { return m_swapchainExtent.height; }

    [[nodiscard]] float extentAspectRatio() const noexcept { return static_cast<float>(m_swapchainExtent.width) / static_cast<float>(m_swapchainExtent.height); }
    VkFormat findDepthFormat();
    VkResult acquireNextImage(std::uint32_t* imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer* buffers, const std::uint32_t* imageIndex);

private:
    Device& device;
    VkExtent2D m_windowExtent;
    VkSwapchainKHR m_swapchain{};

    VkFormat m_swapchainImageFormat{};
    VkExtent2D m_swapchainExtent{};
    std::vector<VkFramebuffer> m_swapchainFramebuffers;
    VkRenderPass m_renderPass{};

    std::vector<VkImage> m_depthImages;
    std::vector<VkDeviceMemory> m_depthImageMemory;
    std::vector<VkImageView> m_depthImageViews;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    std::size_t m_currentFrame{ 0 };

    void createSwapchain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffer();
    void createSyncObjects();

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};

} // !vv

#endif // !SRC_ENGINE_SWAPCHAIN_HPP
