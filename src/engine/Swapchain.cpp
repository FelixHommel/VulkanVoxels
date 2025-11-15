#include "Swapchain.hpp"

#include "Device.hpp"

#include "spdlog/spdlog.h"
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace vv
{

Swapchain::Swapchain(Device& device, VkExtent2D windowExtent)
    : device(device)
    , windowExtent(windowExtent)
{
    createSwapchain();
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
}

Swapchain::Swapchain(Device& device, VkExtent2D windowExtent, std::shared_ptr<Swapchain> previous)
    : device(device)
    , windowExtent(windowExtent)
    , m_oldSwapchain(previous)
{
    createSwapchain();
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
    m_oldSwapchain = nullptr;
}

Swapchain::~Swapchain()
{
    for(const auto& imageView : m_swapchainImageViews)
        vkDestroyImageView(device.device(), imageView, nullptr);
    m_swapchainImageViews.clear();

    if(m_swapchain != nullptr)
    {
        vkDestroySwapchainKHR(device.device(), m_swapchain, nullptr);
        m_swapchain = nullptr;
    }

    for(std::size_t i{ 0 }; i < m_depthImages.size(); ++i)
    {
        vkDestroyImageView(device.device(), m_depthImageViews[i], nullptr);
        vkDestroyImage(device.device(), m_depthImages[i], nullptr);
        vkFreeMemory(device.device(), m_depthImagesMemory[i], nullptr);
    }

    for(const auto& framebuffer : m_swapchainFramebuffers)
        vkDestroyFramebuffer(device.device(), framebuffer, nullptr);

    vkDestroyRenderPass(device.device(), m_renderPass, nullptr);

    for(std::size_t i{ 0 }; i < imageCount(); ++i)
    {
        vkDestroySemaphore(device.device(), m_renderFinishedSemaphores[i], nullptr);
    }

    for(std::size_t i{ 0 }; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(device.device(), m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device.device(), m_inFlightFences[i], nullptr);
    }
}

VkResult Swapchain::acquireNextImage(std::uint32_t* imageIndex)
{
    vkWaitForFences(device.device(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, std::numeric_limits<std::uint64_t>::max());

    return vkAcquireNextImageKHR(device.device(), m_swapchain, std::numeric_limits<std::uint64_t>::max(), m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, imageIndex);
}

VkResult Swapchain::submitCommandBuffer(const VkCommandBuffer* commandBuffer, const std::uint32_t* imageIndex)
{
    if(m_imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(device.device(), 1, &m_imagesInFlight[*imageIndex], VK_TRUE, std::numeric_limits<std::uint64_t>::max());

    m_imagesInFlight[*imageIndex] = m_inFlightFences[m_currentFrame];

    std::array<VkSemaphore, 1> waitSemaphores{ m_imageAvailableSemaphores[m_currentFrame] };
    std::array<VkPipelineStageFlags, 1> waitStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    std::array<VkSemaphore, 1> signalSemaphore{ m_renderFinishedSemaphores[*imageIndex] };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = static_cast<std::uint32_t>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffer;
    submitInfo.signalSemaphoreCount = static_cast<std::uint32_t>(signalSemaphore.size());
    submitInfo.pSignalSemaphores = signalSemaphore.data();

    vkResetFences(device.device(), 1, &m_inFlightFences[m_currentFrame]);
    if(vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("failed to submit command buffer");

    std::array<VkSwapchainKHR, 1> swapchains{ m_swapchain };

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<std::uint32_t>(signalSemaphore.size());
    presentInfo.pWaitSemaphores = signalSemaphore.data();
    presentInfo.swapchainCount = static_cast<std::uint32_t>(swapchains.size());
    presentInfo.pSwapchains = swapchains.data();
    presentInfo.pImageIndices = imageIndex;

    auto result{ vkQueuePresentKHR(device.presentQueue(), &presentInfo) };

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

VkFramebuffer Swapchain::getFramebuffer(std::size_t index) const
{
    if(index >= m_swapchainFramebuffers.size())
        throw std::out_of_range("The element that was tried to access does not exist");

    return m_swapchainFramebuffers[index];
}

/**
 *  Set up the swapchain with format, extent and used queues.
*/
void Swapchain::createSwapchain()
{
    SwapchainSupportDetails swapchainSupport{ device.getSwapchainSupport() };

    VkSurfaceFormatKHR surfaceFormat{ chooseSwapSurfaceFormat(swapchainSupport.formats) };
    VkPresentModeKHR presentMode{ chooseSwapPresentMode(swapchainSupport.presentModes) };
    VkExtent2D extent{ chooseSwapExtent(swapchainSupport.capabilities) };

    std::uint32_t imageCount{ swapchainSupport.capabilities.minImageCount + 1 };
    if(swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
        imageCount = swapchainSupport.capabilities.maxImageCount;
    spdlog::info("using {} swap images", imageCount);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = device.surface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = m_oldSwapchain == nullptr ? VK_NULL_HANDLE : m_oldSwapchain->m_swapchain;

    QueueFamilyIndices indices{ device.findPhysicalQueueFamilies() };
    if(indices.graphicsFamily.value() != indices.presentFamily.value())
    {
        std::array<std::uint32_t, 2> queueFamilyIndices{ indices.presentFamily.value(), indices.graphicsFamily.value() };

        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = static_cast<std::uint32_t>(queueFamilyIndices.size());
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }

    if(vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
        throw std::runtime_error("failed to create swapchain");

    vkGetSwapchainImagesKHR(device.device(), m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device.device(), m_swapchain, &imageCount, m_swapchainImages.data());

    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainImageExtent = extent;
}

/**
 *  Set up the image views used by the swapchain.
*/
void Swapchain::createImageViews()
{
    m_swapchainImageViews.resize(m_swapchainImages.size());

    for(std::size_t i{0}; i < m_swapchainImageViews.size(); ++i)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapchainImageFormat;
        createInfo.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        if(vkCreateImageView(device.device(), &createInfo, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create image view");
    }
}

/**
 *  Set up the render passes with all used attachments.
*/
void Swapchain::createRenderPass()
{
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments{ colorAttachment, depthAttachment };
    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    createInfo.pAttachments = attachments.data();
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &dependency;

    if(vkCreateRenderPass(device.device(), &createInfo, nullptr, &m_renderPass) != VK_SUCCESS)
        throw std::runtime_error("failed to create render pass");
}

/**
 *  Set up the depth images, image views and their memory on the device.
*/
void Swapchain::createDepthResources()
{
    VkFormat depthFormat{ findDepthFormat() };
    VkExtent2D swapchainExtent{ m_swapchainImageExtent };

    m_depthImages.resize(imageCount());
    m_depthImagesMemory.resize(imageCount());
    m_depthImageViews.resize(imageCount());

    for(std::size_t i{0}; i < imageCount(); ++i)
    {
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = depthFormat;
        imageCreateInfo.extent = {
            .width = swapchainExtent.width,
            .height = swapchainExtent.height,
            .depth = 1
        };
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        device.createImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImages[i], m_depthImagesMemory[i]);

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = m_depthImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = depthFormat;
        imageViewCreateInfo.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        if(vkCreateImageView(device.device(), &imageViewCreateInfo, nullptr, &m_depthImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to depth resources");
    }
}

/**
 *  Set up the framebuffers which are use by the swapchain to display frames.
*/
void Swapchain::createFramebuffers()
{
    m_swapchainFramebuffers.resize(imageCount());

    for(std::size_t i{0}; i < imageCount(); ++i)
    {
        std::array<VkImageView, 2> attachments{ m_swapchainImageViews[i], m_depthImageViews[i] };
        VkExtent2D swapchainExtent{ m_swapchainImageExtent };

        VkFramebufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = m_renderPass;
        createInfo.attachmentCount = static_cast<std::uint32_t>(attachments.size());
        createInfo.pAttachments = attachments.data();
        createInfo.width = swapchainExtent.width;
        createInfo.height = swapchainExtent.height;
        createInfo.layers = 1;

        if(vkCreateFramebuffer(device.device(), &createInfo, nullptr, &m_swapchainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer");
    }
}

/**
 *  Set up the semaphores and fences used during the presentation of frames.
*/
void Swapchain::createSyncObjects()
{
    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(imageCount());
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    m_imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(std::size_t i{0}; i < m_renderFinishedSemaphores.size(); ++i)
    {
        if(vkCreateSemaphore(device.device(), &semaphoreCreateInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create semaphore");
    }

    for(std::size_t i{0}; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if(vkCreateSemaphore(device.device(), &semaphoreCreateInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device.device(), &fenceCreateInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create sync objects");
    }
}

/**
 *  Choose a Extent2D for the swapchain.
 *
 *  @param capabilities - VkSurfaceCapabilitiesKHR object containing details about the extent of the surface
 *  @returns VkExtent2D - extent that is going to be used by the swapchain
*/
VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
    if(capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;

    VkExtent2D actualExtent{ windowExtent };
    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.minImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.minImageExtent.height, actualExtent.height));

    return actualExtent;
}

/**
 *  Search the device for a suitable depth format
 *
 *  @returns VkFormat - which is going to be used by the depth resources
*/
VkFormat Swapchain::findDepthFormat() const
{
    return device.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

/**
 *  Pick a suitable Surface format from a selection of available formats
 *
 *  @param availableFormats - vector with all available Formats
 *  @return the first format that matches VK_FORMAT_B8G8R8A8_UNORM or if there is no format that matches,
 *          the first format in <code>availableFormats<\code>
*/
VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for(const auto& format : availableFormats)
    {
        if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }

    return availableFormats.at(0);
}

/**
 *  Pick a present mode for the swapchain.
 *
 *  @param availablePresentModes - vector containing all available present modes
 *  @returns present mode with Mailbox system, if none match return FIFO(V-Sync)
*/
VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for(const auto& presentMode : availablePresentModes)
    {
        if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            spdlog::info("Picked MAILBOX present mode");
            return presentMode;
        }
    }

    spdlog::info("Picked V-Sync present mode");
    return VK_PRESENT_MODE_FIFO_KHR;
}

} // !vv
