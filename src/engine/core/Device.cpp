#include "Device.hpp"

#include "core/Window.hpp"
#include "utility/exceptions/Exception.hpp"
#include "utility/exceptions/VulkanException.hpp"

#include "GLFW/glfw3.h"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"
#include "vk_mem_alloc.h"
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/
)
{
    switch(severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        spdlog::log(spdlog::level::trace, "{}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        spdlog::log(spdlog::level::info, "{}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        spdlog::log(spdlog::level::warn, "{}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        spdlog::log(spdlog::level::err, "{}", pCallbackData->pMessage);
        break;
    default:
        spdlog::log(spdlog::level::critical, "{}", pCallbackData->pMessage);
        break;
    }

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
)
{
    const auto func{ reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
    ) };

    if(func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* pAllocator
)
{
    const auto func{ reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
    ) };

    if(func != nullptr)
        func(instance, debugMessenger, pAllocator);
}

} // namespace

namespace vv
{

Device::Device(std::shared_ptr<Window> window) : window{ std::move(window) }
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createAllocator();
    createCommandPool();
}

Device::Device(bool headless) : window{ nullptr }, m_headless{ headless }
{
#if defined(VV_ENABLE_ASSERTS)
    assert(headless && "Use the other constructor to use for windowed rednering");
#endif

    createInstance();
    pickPhysicalDevice();
    createLogicalDevice();
    createAllocator();
    createCommandPool();
}

Device::~Device()
{
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vmaDestroyAllocator(m_allocator);
    vkDestroyDevice(m_device, nullptr);

    if constexpr(ENABLE_VALIDATION_LAYERS)
        ::DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);

    if(window != nullptr)
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

    vkDestroyInstance(m_instance, nullptr);
}

std::uint32_t Device::findMemoryType(const std::uint32_t filter, const VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for(std::uint32_t i{ 0 }; i < memProperties.memoryTypeCount; ++i)
    {
        // NOLINTNEXTLINE(*-pro-bounds-constant-array-index): Vulkan API requires accessing C-style arrays by index
        if((filter & (1 << i)) != VK_FALSE && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw Exception("Failed to find a suitable memory type");
}

VkFormat Device::findSupportedFormat(
    const std::vector<VkFormat>& candidates, const VkImageTiling tiling, const VkFormatFeatureFlags features
) const
{
    for(const auto format : candidates)
    {
        VkFormatProperties properties{};
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &properties);

        if(tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
            return format;

        if(tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
            return format;
    }

    throw Exception("Failed to find a supported format");
}

void Device::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    const VmaAllocationCreateInfo& allocInfo,
    VkBuffer& buffer,
    VmaAllocation& allocation
) const
{
    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = size;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    const VkResult result{ vmaCreateBuffer(m_allocator, &createInfo, &allocInfo, &buffer, &allocation, nullptr) };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to allocate buffer", result);
}

VkCommandBuffer Device::beginSingleTimeCommand() const
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer{};
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Device::endSingleTimeCommand(VkCommandBuffer commandBuffer) const
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
{
    VkCommandBuffer commandBuffer{ beginSingleTimeCommand() };

    const VkBufferCopy copyRegion{ .srcOffset = 0, .dstOffset = 0, .size = size };

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommand(commandBuffer);
}

void Device::copyBufferToImage(
    VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height, std::uint32_t layerCount
) const
{
    VkCommandBuffer commandBuffer{ beginSingleTimeCommand() };

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource
        = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = layerCount };
    region.imageOffset = { .x = 0, .y = 0, .z = 0 };
    region.imageExtent = { .width = width, .height = height, .depth = 1 };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommand(commandBuffer);
}

void Device::createImage(
    const VkImageCreateInfo& imageInfo,
    VkImage& image,
    VmaAllocation& allocation
) const
{
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    const VkResult result{ vmaCreateImage(m_allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr) };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to create image", result);
}

void Device::createImageWithInfo(
    const VkImageCreateInfo& imageInfo,
    const VkMemoryPropertyFlags properties,
    VkImage& image,
    VkDeviceMemory& imageMemory
) const
{
    VkResult result{ vkCreateImage(m_device, &imageInfo, nullptr, &image) };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to create image", result);

    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements(m_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory);
    if(result != VK_SUCCESS)
    {
        throw VulkanException("Failed to allocate image memory", result);
    }

    result = vkBindImageMemory(m_device, image, imageMemory, 0);
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to bind image memory", result);
}

void Device::createInstance()
{
    if(ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport())
        throw Exception("Validation layers enabled but not available");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanVoxels";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VULKAN_VERSION;

    const auto extensions{ getRequiredExtensions() };

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;
    createInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

#if defined(__APPLE__)
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
    if constexpr(ENABLE_VALIDATION_LAYERS)
    {
        createInfo.enabledLayerCount = static_cast<std::uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

        populateDebugMessengerCreateInfo(debugMessengerCreateInfo);
        createInfo.pNext = &debugMessengerCreateInfo;
    }

    const VkResult result{ vkCreateInstance(&createInfo, nullptr, &m_instance) };
    if(result != 0)
        throw VulkanException("Failed to create vulkan instance", result);

    if(!m_headless)
        hasGlfwRequiredInstanceExtensions();
}

void Device::setupDebugMessenger()
{
    if constexpr(!ENABLE_VALIDATION_LAYERS)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    const VkResult result{ ::CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to create debug messenger", result);
}

void Device::createSurface()
{
    window->createWindowSurface(m_instance, &m_surface);
}

void Device::pickPhysicalDevice()
{
    std::uint32_t deviceCount{};
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if(deviceCount == 0)
        throw Exception("Failed to find any GPUs that support vulkan");

    spdlog::info("Found {} devices", deviceCount);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    for(const auto& device : devices)
    {
        if(isDeviceSuitable(device))
        {
            m_physicalDevice = device;
            break;
        }
    }

    if(m_physicalDevice == VK_NULL_HANDLE)
        throw Exception("Failed to find a suitable GPU");

    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

    spdlog::info("Chosen physical device: {}", properties.deviceName);
}

void Device::createLogicalDevice()
{
    const QueueFamilyIndices indices{ findQueueFamilies(m_physicalDevice) };

    if(!indices.graphicsFamily.has_value() || !indices.presentFamily.has_value())
        throw Exception("Failed to find queues");

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const std::set<std::uint32_t> uniqueQueueFamilies{ indices.graphicsFamily.value(), indices.presentFamily.value() };

    constexpr float queuePriority{ 1.f };
    for(std::uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledLayerCount = 0;
    createInfo.pEnabledFeatures = &deviceFeatures;

    // NOTE: Headless mode does not need swapchain extensions
    if(!m_headless)
    {
        createInfo.enabledExtensionCount = static_cast<std::uint32_t>(DEVICE_EXTENSIONS.size());
        createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
    }

    // NOTE: Device specific validation layers have been deprecated. Potentially remove later
    if constexpr(ENABLE_VALIDATION_LAYERS)
    {
        createInfo.enabledLayerCount = static_cast<std::uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    }

    const VkResult result{ vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to create logical device", result);

    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);

    if(!m_headless)
        vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
    else
        m_presentQueue = m_graphicsQueue;
}

void Device::createAllocator()
{
    VmaAllocatorCreateInfo createInfo = {};
    createInfo.instance = m_instance;
    createInfo.physicalDevice = m_physicalDevice;
    createInfo.device = m_device;
    createInfo.vulkanApiVersion = VULKAN_VERSION;

    const VkResult result{ vmaCreateAllocator(&createInfo, &m_allocator) };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to create allocator", result);
}

void Device::createCommandPool()
{
    const QueueFamilyIndices indices{ findPhysicalQueueFamilies() };

    if(!indices.graphicsFamily.has_value())
        throw Exception("Failed to find graphics queue family");

    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = indices.graphicsFamily.value();

    const VkResult result{ vkCreateCommandPool(m_device, &createInfo, nullptr, &m_commandPool) };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to create command pool", result);
}

bool Device::isDeviceSuitable(VkPhysicalDevice phDevice) const
{
    QueueFamilyIndices indices{ findQueueFamilies(phDevice) };

    bool extensionsSupported{ checkDeviceExtensionSupport(phDevice) };

    bool swapchainAdequate{ false };

    if(extensionsSupported && !m_headless)
    {
        SwapchainSupportDetails swapchainSupport{ querySwapchainSupport(phDevice) };
        swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
    }
    else if(m_headless)
    {
        // NOTE: surface being null implies headless mode, in which the swapchain doesn't matter
        swapchainAdequate = true;
    }

    VkPhysicalDeviceFeatures supportedFeatures{};
    vkGetPhysicalDeviceFeatures(phDevice, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapchainAdequate
        && (supportedFeatures.samplerAnisotropy != VK_FALSE);
}

std::vector<const char*> Device::getRequiredExtensions()
{
    std::uint32_t glfwExtensionCount{ 0 };
    const char** glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

    const std::span<const char*> extensionSpan(glfwExtensions, glfwExtensionCount);
    std::vector<const char*> extensions(extensionSpan.cbegin(), extensionSpan.cend());

    if constexpr(ENABLE_VALIDATION_LAYERS)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#if defined(__APPLE__)
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
    }

    return extensions;
}

bool Device::checkValidationLayerSupport() const
{
    std::uint32_t layerCount{};
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char* layerName : VALIDATION_LAYERS)
    {
        bool layerFound{ false };

        for(const auto& layerProperties : availableLayers)
        {
            if(std::strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if(!layerFound)
            return false;
    }

    return true;
}

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice phDevice) const
{
    QueueFamilyIndices indices{};

    std::uint32_t queueFamilyCount{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(phDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(phDevice, &queueFamilyCount, queueFamilies.data());

    for(std::uint32_t i{ 0 }; const auto& queueFamily : queueFamilies)
    {
        if(queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != VK_FALSE)
            indices.graphicsFamily = i;

        if(!m_headless)
        {
            VkBool32 presentSupport{ VK_FALSE };
            vkGetPhysicalDeviceSurfaceSupportKHR(phDevice, i, m_surface, &presentSupport);

            if(queueFamily.queueCount > 0 && presentSupport != VK_FALSE)
                indices.presentFamily = i;
        }
        else
        {
            // NOTE: In headless mode the present queue doesn't matter, just use the graphics queue
            indices.presentFamily = indices.graphicsFamily;
        }

        if(indices.isComplete())
            break;

        ++i;
    }

    return indices;
}

void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity
        = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                           | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                           | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = ::debugCallback;
}

void Device::hasGlfwRequiredInstanceExtensions()
{
    std::uint32_t extensionCount{ 0 };
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    spdlog::log(spdlog::level::info, "available extensions:");
    std::unordered_set<std::string> available{};
    for(const auto& extension : extensions)
    {
        spdlog::log(spdlog::level::info, "\t{}", extension.extensionName);
        available.insert(extension.extensionName);
    }

    spdlog::info("Required extensions:");
    const auto requiredExtensions{ getRequiredExtensions() };
    for(const auto& required : requiredExtensions)
    {
        spdlog::log(spdlog::level::info, "\t{}", required);

        if(!available.contains(required))
            throw std::runtime_error("Missing required glfw extension");
    }
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice phDevice) const
{
    std::uint32_t extensionCount{};
    vkEnumerateDeviceExtensionProperties(phDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(phDevice, nullptr, &extensionCount, availableExtensions.data());
    std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.cbegin(), DEVICE_EXTENSIONS.cend());

    for(const auto& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

SwapchainSupportDetails Device::querySwapchainSupport(VkPhysicalDevice phDevice) const
{
    SwapchainSupportDetails details{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phDevice, m_surface, &details.capabilities);

    std::uint32_t formatCount{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(phDevice, m_surface, &formatCount, nullptr);

    if(formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(phDevice, m_surface, &formatCount, details.formats.data());
    }

    std::uint32_t presentModeCount{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(phDevice, m_surface, &presentModeCount, nullptr);

    if(presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(phDevice, m_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

} // namespace vv
