#ifndef VULKAN_VOXELS_SRC_ENGINE_CORE_TEXTURE_2D_HPP
#define VULKAN_VOXELS_SRC_ENGINE_CORE_TEXTURE_2D_HPP

#include "core/Device.hpp"

#include "vk_mem_alloc.h"
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <filesystem>
#include <memory>

namespace vv
{

/// \brief Abstraction over Textures for easier usage with Vulkan
///
/// \author Felix Hommel
/// \date 12/15/12025
class Texture2D
{
public:
    /// \brief Create a new 2D Texture
    ///
    /// \param device the \ref VkDevice wehre the image is created on
    /// \param filepath the path to the texture image
    Texture2D(std::shared_ptr<Device> device, const std::filesystem::path& filepath);
    ~Texture2D();

    Texture2D(const Texture2D&) = delete;
    Texture2D(Texture2D&& other) noexcept;
    Texture2D& operator=(const Texture2D&) = delete;
    Texture2D& operator=(Texture2D&&) = delete;

    [[nodiscard]] VkImage image() const noexcept { return m_image; }
    [[nodiscard]] VkDescriptorImageInfo descriptor() const noexcept { return m_descriptor; }

    /// \brief update the descriptor information
    void updateDescriptor() noexcept;

private:
    std::shared_ptr<Device> device;

    VkImage m_image{ VK_NULL_HANDLE };
    VkImageView m_imageView{ VK_NULL_HANDLE };
    VmaAllocation m_allocation{ VK_NULL_HANDLE };
    VkSampler m_sampler{ VK_NULL_HANDLE };
    VkDescriptorImageInfo m_descriptor{};

    std::uint32_t m_width{ 0 };
    std::uint32_t m_height{ 0 };
    std::uint32_t m_mipLevels{ 0 };
    VkFormat m_format{ VK_FORMAT_R8G8B8A8_SRGB };

    void loadFromFile(const std::filesystem::path& filepath);
    void generateMipMaps();
    void createImageView();
    void createSampler();

    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_CORE_TEXTURE_2D_HPP

