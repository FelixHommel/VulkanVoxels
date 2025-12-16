#ifndef VULKAN_VOXELS_SRC_ENGINE_CORE_TEXTURE_2D_HPP
#define VULKAN_VOXELS_SRC_ENGINE_CORE_TEXTURE_2D_HPP

#include "core/Device.hpp"

#include "vk_mem_alloc.h"
#include <vulkan/vulkan_core.h>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>

namespace vv
{

/// \brief
///
/// \author Felix Hommel
/// \date 12/15/2025
struct TextureConfig
{
public:
    VkFormat format{ VK_FORMAT_R8G8B8A8_SRGB };
    VkFilter minFilter{ VK_FILTER_LINEAR };
    VkFilter magFilter{ VK_FILTER_LINEAR };
    VkSamplerAddressMode addressMode{ VK_SAMPLER_ADDRESS_MODE_REPEAT };
    VkSamplerMipmapMode mipmapMode{ VK_SAMPLER_MIPMAP_MODE_LINEAR };
    // TODO: Implement functionality of these flags. Currently non-functional (mipmaps and anisotropy alwasy on)
    bool mipmapsEnable{ true };
    bool anisotropyEnable{ true };

    static TextureConfig albedo() { return {}; }

    static TextureConfig normal()
    {
        TextureConfig config{};
        config.format = VK_FORMAT_R8G8B8A8_UNORM;

        return config;
    }

    static TextureConfig hdr()
    {
        TextureConfig config{};
        config.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        config.mipmapsEnable = false;

        return config;
    }
};

/// \brief Abstraction over Textures for easier usage with Vulkan
///
/// \author Felix Hommel
/// \date 12/15/12025
class Texture2D
{
public:
    static Texture2D loadFromFile(
        std::shared_ptr<Device> device, const std::filesystem::path& filepath, const TextureConfig& config
    );

    /// \brief Create a new 2D Texture
    ///
    /// \param device the \ref VkDevice where the image is created on
    /// \param width the width of the image
    /// \param height the height of the image
    /// \param config \ref TextureConfig containing configuring information for the texture
    /// \param pixels (optional) the raw image data
    Texture2D(
        std::shared_ptr<Device> device,
        std::uint32_t width,
        std::uint32_t height,
        const TextureConfig& config,
        std::span<const std::byte> pixels = {}
    );
    ~Texture2D();

    Texture2D(const Texture2D&) = delete;
    Texture2D(Texture2D&& other) noexcept;
    Texture2D& operator=(const Texture2D&) = delete;
    Texture2D& operator=(Texture2D&&) = delete;

    [[nodiscard]] VkImage image() const noexcept { return m_image; }
    [[nodiscard]] VkDescriptorImageInfo descriptor() const noexcept { return m_descriptor; }
    [[nodiscard]] VkFormat format() const noexcept { return m_config.format; }
    [[nodiscard]] std::uint32_t width() const noexcept { return m_width; }
    [[nodiscard]] std::uint32_t height() const noexcept { return m_height; }
    [[nodiscard]] std::uint32_t mipLevels() const noexcept { return m_mipLevels; }

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
    std::uint32_t m_mipLevels{ 1 };
    TextureConfig m_config;

    void uploadImageData(std::span<const std::byte> pixels);
    void generateMipMaps();
    void createImageView();
    void createSampler();

    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_CORE_TEXTURE_2D_HPP

