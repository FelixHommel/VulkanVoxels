#include "Device.hpp"

#include "GLFW/glfw3.h"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

#include <cstring>
#include <set>
#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace
{
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* /*pUserData*/)
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
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        const auto func{ reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")) };

        if(func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessengerEXT(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator)
    {
        const auto func{ reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")) };

        if(func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }
}

namespace vv
{

Device::Device(Window& window)
    : window{ window }
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
}

Device::~Device()
{
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vkDestroyDevice(m_device, nullptr);

    if constexpr(ENABLE_VALIDATION_LAYERS)
        ::DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

std::uint32_t Device::findMemoryType(std::uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for(std::uint32_t i{ 0 }; i < memProperties.memoryTypeCount; ++i)
    {
        if((typeFilter & (1 << i)) != VK_FALSE && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) //NOLINT
            return i;
    }

    throw std::runtime_error("failed to find a suitable memoty type");
}

VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
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

    throw std::runtime_error("failed to find a supported format");
}

void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = size;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(m_device, &createInfo, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create vertex buffer");

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if(vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        throw std::runtime_error(" failed to allocate vertex buffer memory");

    vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}

VkCommandBuffer Device::beginSingleTimeCommand()
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

void Device::endSingleTimeCommand(VkCommandBuffer commandBuffer)
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

void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    const VkCommandBuffer commandBuffer{beginSingleTimeCommand()};

    const VkBufferCopy copyRegion{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommand(commandBuffer);
}

void Device::copyBufferToImage(VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height, std::uint32_t layerCount)
{
    const VkCommandBuffer commandBuffer{ beginSingleTimeCommand() };

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = layerCount
    };
    region.imageOffset = { .x = 0, .y = 0, .z = 0 };
    region.imageExtent = { .width = width, .height = height, .depth = 1 };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommand(commandBuffer);
}

void Device::createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    if(vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS)
        throw std::runtime_error("failed to create image");

    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements(m_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if(vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory");

    if(vkBindImageMemory(m_device, image, imageMemory, 0) != VK_SUCCESS)
        throw std::runtime_error("failed to bind image memory");
}

void Device::createInstance()
{
    if(ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport())
        throw std::runtime_error("validation layers enabled, but not available");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanVoxels";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_4;

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

    if(vkCreateInstance(&createInfo, nullptr, &m_instance) != 0)
        throw std::runtime_error("failed to create vulkan instance");

    hasGlfwRequiredInstanceExtensions();
}

void Device::setupDebugMessenger()
{
    if constexpr(!ENABLE_VALIDATION_LAYERS)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    if(::CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
        throw std::runtime_error("failed to create debug messenger");
}

void Device::createSurface()
{
    window.createWindowSurface(m_instance, &m_surface);
}

void Device::pickPhysicalDevice()
{
    std::uint32_t deviceCount{};
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if(deviceCount == 0)
        throw std::runtime_error("failed to find any GPUs that support vulkan");

    spdlog::log(spdlog::level::info, "Found {} devices", deviceCount);

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
        throw std::runtime_error("failed to find a suitable GPU");

    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
    spdlog::log(spdlog::level::info, "Chosen physical device: {}", properties.deviceName);
}

void Device::createLogicalDevice()
{
    const QueueFamilyIndices indices{ findQueueFamilies(m_physicalDevice) };

    if(!indices.isComplete())
        throw std::runtime_error("failed to find queues");

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const std::set<std::uint32_t> uniqueQueueFamilies{ indices.graphicsFamily.value(), indices.presentFamily.value() };

    const float queuePriority{ 1.f };
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
    createInfo.enabledExtensionCount = static_cast<std::uint32_t>(DEVICE_EXTENSIONS.size());
    createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    // NOTE: Device specific validation layers have been deprecated. Potentially remove later
    if constexpr(ENABLE_VALIDATION_LAYERS)
    {
        createInfo.enabledLayerCount = static_cast<std::uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    }

    if(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device");

    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
}

void Device::createCommandPool()
{
    const QueueFamilyIndices queueFamilyIndices{ findPhysicalQueueFamilies() };

    if(!queueFamilyIndices.graphicsFamily.has_value())
        throw std::runtime_error("failed to find graphics queue family");

    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if(vkCreateCommandPool(m_device, &createInfo, nullptr, &m_commandPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool");
}

bool Device::isDeviceSuitable(VkPhysicalDevice phDevice)
{
    QueueFamilyIndices indices{ findQueueFamilies(phDevice) };

    bool extensionsSupported{ checkDeviceExtensionSupport(phDevice) };

    bool swapchainAdequate{ false };
    if(extensionsSupported)
    {
        SwapchainSupportDetails swapchainSupport{ querySwapchainSupport(phDevice) };
        swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures{};
    vkGetPhysicalDeviceFeatures(phDevice, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapchainAdequate && (supportedFeatures.samplerAnisotropy != VK_FALSE);
}

std::vector<const char*> Device::getRequiredExtensions()
{
    std::uint32_t glfwExtensionCount{ 0 };
    const char** glfwExtensions{
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount)
    };

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

bool Device::checkValidationLayerSupport()
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

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice phDevice)
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

        VkBool32 presentSupport{ VK_FALSE };
        vkGetPhysicalDeviceSurfaceSupportKHR(phDevice, i, m_surface, &presentSupport);

        if(queueFamily.queueCount > 0 && presentSupport != VK_FALSE)
            indices.presentFamily = i;

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
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
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

    spdlog::log(spdlog::level::info, "rquired extensions:");
    const auto requiredExtensions{ getRequiredExtensions() };
    for(const auto& required : requiredExtensions)
    {
        spdlog::log(spdlog::level::info, "\t{}", required);

        if(!available.contains(required))
            throw std::runtime_error("Missing required glfw extension");
    }
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice phDevice)
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

SwapchainSupportDetails Device::querySwapchainSupport(VkPhysicalDevice phDevice)
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

} // !vv
