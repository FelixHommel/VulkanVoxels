#ifndef SRC_ENGINE_DEVICE_HPP
#define SRC_ENGINE_DEVICE_HPP

#include "Window.hpp"

#include <cstdint>
#include <optional>
#include <vulkan/vulkan_core.h>

#include <vector>

namespace vv
{

/// \brief Save information of what the swapchain is supporting
///
/// \author Felix Hommel
/// \date 11/10/2025
struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

/// \brief Save the indices of the used queues
///
/// \author Felix Hommel
/// \date 11/10/2025
struct QueueFamilyIndices
{
    std::optional<std::uint32_t> graphicsFamily;
    std::optional<std::uint32_t> presentFamily;

    [[nodiscard]] constexpr bool isComplete() const noexcept { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

/// \brief The Device is a logical abstraction over the physical GPU
///
/// \author Felix Hommel
/// \date 11/10/2025
class Device
{
public:
#ifdef DEBUG
    static constexpr bool ENABLE_VALIDATION_LAYERS{ true };
#else
    static constexpr bool ENABLE_VALIDATION_LAYERS{ false };
#endif

    Device(Window& window);
    ~Device();

    Device(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(const Device&) = delete;
    Device& operator=(Device&&) = delete;

    VkCommandPool commandPool() { return m_commandPool; }
    VkDevice device() { return m_device; }
    VkSurfaceKHR surface() { return m_surface; }
    VkQueue graphicsQueue() { return m_graphicsQueue; }
    VkQueue presentQueue() { return m_presentQueue; }

    /// \brief Query the physical device for its swapchain support
    ///
    /// \return \ref SwapchainSupportDetails the details of the supported swapchain features
    SwapchainSupportDetails getSwapchainSupport() { return querySwapchainSupport(m_physicalDevice); }
    /// \brief Find a suitable memory type from the physical device
    ///
    /// \param typeFilter filter out specific unwanted types
    /// \param properties properties the memory needs to fulfill
    ///
    /// \return std::uint32_t 
    std::uint32_t findMemoryType(std::uint32_t typeFilter, VkMemoryPropertyFlags properties);
    /// \brief Find apropriate queues on the physical device
    ///
    /// \return \ref QueueFamilyIndices the chosen queues
    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(m_physicalDevice); }
    /// \brief Determine the best fitting format from a selection
    ///
    /// \param candidates proposed candidates that are available to be the format
    /// \param tiling which tiling the format has
    /// \param features which features the format needs to support
    ///
    /// \return \ref VkFormat the chosen format
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    /// \brief Create a new Buffer
    ///
    /// Create the new buffer, allocate memory for it and map the memory regions from the physical device to the CPU
    ///
    /// \param size how bug the buffer is
    /// \param usage how the buffer is going to be used
    /// \param properties memory properties for the buffer memory
    /// \param buffer where to store the handle to the new buffer
    /// \param bufferMemory handle to the memory region
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    /// \brief Start recording a command
    ///
    /// \return \ref VkCommandBuffer handle to the command buffer that is being recorded to
    VkCommandBuffer beginSingleTimeCommand();
    /// \brief End recording a command
    ///
    /// \param \ref VkCommandBuffer which command buffer should stop recording
    void endSingleTimeCommand(VkCommandBuffer commandBuffer);
    /// \brief Copy data from one buffer to another
    ///
    /// \param src buffer to copy from
    /// \param dst buffer to copy to
    /// \param size how big the copied data is
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    /// \brief Copy buffer data to an image
    ///
    /// \param buffer the buffer to copy from
    /// \param image the image to copy to
    /// \param width the width of the image
    /// \param height the height of the image
    /// \param layerCount how many layers the image has
    void copyBufferToImage(VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height, std::uint32_t layerCount);

    /// \brief Create a new Image
    ///
    /// Create a new image, allocate memory for it and map the memory regions from physical device to the CPU
    ///
    /// \param imageInfo \ref VkImageCreateInfo struct with the required information filled out
    /// \param properties properties the memory of the image needs to fulfill
    /// \param image where to store the iamge handle to
    /// \param imageMemory where to store the image memory handle to
    void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    VkPhysicalDeviceProperties properties{}; // NOLINT

private:
    const std::vector<const char*> VALIDATION_LAYERS{ "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> DEVICE_EXTENSIONS{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkInstance m_instance{ VK_NULL_HANDLE };
    VkDebugUtilsMessengerEXT m_debugMessenger{ VK_NULL_HANDLE };
    VkPhysicalDevice m_physicalDevice{ VK_NULL_HANDLE };
    VkCommandPool m_commandPool{ VK_NULL_HANDLE };
    VkDevice m_device{ VK_NULL_HANDLE };
    VkSurfaceKHR m_surface{ VK_NULL_HANDLE };
    VkQueue m_graphicsQueue{ VK_NULL_HANDLE };
    VkQueue m_presentQueue{ VK_NULL_HANDLE };

    Window& window;

    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    bool isDeviceSuitable(VkPhysicalDevice phDevice);
    static std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice phDevice);
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static void hasGlfwRequiredInstanceExtensions();
    bool checkDeviceExtensionSupport(VkPhysicalDevice phDevice);
    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice phDevice);
};

} //!vv

#endif //!SRC_ENGINE_DEVICE_HPP
