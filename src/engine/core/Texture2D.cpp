#include "Texture2D.hpp"

#include "core/Buffer.hpp"
#include "utility/exceptions/Exception.hpp"
#include "utility/exceptions/VulkanException.hpp"

#include "external/stb_image.h"
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <utility>

namespace vv
{

Texture2D::Texture2D(std::shared_ptr<Device> device, const std::filesystem::path& filepath)
    : device(std::move(device))
{
    loadFromFile(filepath);
    generateMipMaps();
    createImageView();
    createSampler();
    
    updateDescriptor();
}

Texture2D::~Texture2D()
{
    vkDestroyImageView(device->device(), m_imageView, nullptr);
    vmaDestroyImage(device->allocator(), m_image, m_allocation);
    if(m_sampler != VK_NULL_HANDLE)
        vkDestroySampler(device->device(), m_sampler, nullptr);
}

Texture2D::Texture2D(Texture2D&& other) noexcept
    : device(std::move(other.device))
    , m_image(other.m_image)
    , m_imageView(other.m_imageView)
    , m_allocation(other.m_allocation)
    , m_sampler(other.m_sampler)
    , m_descriptor(other.m_descriptor)
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_mipLevels(other.m_mipLevels)
{
    other.m_image = VK_NULL_HANDLE;
    other.m_imageView = VK_NULL_HANDLE;
    other.m_allocation = VK_NULL_HANDLE;
    other.m_sampler = VK_NULL_HANDLE;
    other.m_descriptor = {};

    other.m_width = 0;
    other.m_height = 0;
    other.m_mipLevels = 0;
}

void Texture2D::updateDescriptor() noexcept
{
    m_descriptor.sampler = m_sampler;
    m_descriptor.imageView = m_imageView;
    m_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

/// \brief Load the image data from the file
///
/// \param filepath path to the image texture
void Texture2D::loadFromFile(const std::filesystem::path& filepath)
{
    int texWidth{ 0 };
    int texHeight{ 0 };
    int texChannels{ 0 };
    stbi_uc* pixels{ stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha) };
    m_width = static_cast<std::uint32_t>(texWidth);
    m_height = static_cast<std::uint32_t>(texHeight);
    m_mipLevels = static_cast<std::uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if(pixels == nullptr)
        throw Exception("Failed to load texture image");

    VkDeviceSize imageSize{ static_cast<VkDeviceSize>(texWidth * texHeight) * 4 };
    auto stagingBuffer{ Buffer::createStagingBuffer(this->device, imageSize, 1) };

    stagingBuffer.writeToBufferRaw(pixels, imageSize);
    stagingBuffer.flush();
    stbi_image_free(pixels);
    
    VkImageCreateInfo imageCI{};
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.extent = {
        .width = m_width,
        .height = m_height,
        .depth = 1
    };
    imageCI.mipLevels = m_mipLevels;
    imageCI.arrayLayers = 1;
    imageCI.format = m_format;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCI.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;

    device->createImage(imageCI, m_image, m_allocation);
    transitionImageLayout(m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    device->copyBufferToImage(stagingBuffer.getBuffer(), m_image, m_width, m_height, 1);
}

/// \brief generate the mipmap levels for the texture
void Texture2D::generateMipMaps()
{
    VkFormatProperties formatProperties{};
    vkGetPhysicalDeviceFormatProperties(device->physicalDevice(), m_format, &formatProperties);

    if((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0u)
        throw Exception("Texture image format does not support linear blitting");

    VkCommandBuffer commandBuffer{ device->beginSingleTimeCommand() };

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    std::int32_t mipWidth{ static_cast<std::int32_t>(m_width) };
    std::int32_t mipHeight{ static_cast<std::int32_t>(m_height) };

    for(std::uint32_t i{ 1 }; i < m_mipLevels; ++i)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = { .x = 0, .y = 0, .z = 0 };
        blit.srcOffsets[1] = { .x = mipWidth, .y = mipHeight, .z = 1 };
        blit.srcSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = i - 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };
        blit.dstOffsets[0] = { .x = 0, .y = 0, .z = 0 };
        blit.dstOffsets[1] = {
            .x = mipWidth > 1 ? mipWidth / 2 : 1,
            .y = mipHeight > 1 ? mipHeight / 2 : 1,
            .z = 1
        };
        blit.dstSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = i,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        vkCmdBlitImage(commandBuffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        if(mipWidth > 1)
            mipWidth /= 2;
        if(mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = m_mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    device->endSingleTimeCommand(commandBuffer);
}

/// \brief Create the image view for the texture
void Texture2D::createImageView()
{
    VkImageViewCreateInfo imageViewCI{};
    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCI.image = m_image;
    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCI.format = m_format;
    imageViewCI.subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = m_mipLevels,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    const VkResult result{ vkCreateImageView(device->device(), &imageViewCI, nullptr, &m_imageView) };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to create texture image view", result);
}

/// \brief Create the sampler for the texture
void Texture2D::createSampler()
{
    VkSamplerCreateInfo samplerCI{};
    samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCI.magFilter = VK_FILTER_LINEAR;
    samplerCI.minFilter = VK_FILTER_LINEAR;
    samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCI.anisotropyEnable = VK_TRUE;
    samplerCI.maxAnisotropy = device->properties.limits.maxSamplerAnisotropy;
    samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCI.unnormalizedCoordinates = VK_FALSE;
    samplerCI.compareEnable = VK_FALSE;
    samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCI.minLod = 0.f;
    samplerCI.maxLod = static_cast<float>(m_mipLevels);
    samplerCI.mipLodBias = 0.f;

    const VkResult result{ vkCreateSampler(device->device(), &samplerCI, nullptr, &m_sampler) };
    if(result != VK_SUCCESS)
        throw VulkanException("Failed to create texture sampler", result);
}

/// \brief Transition a imgae layout from one to another
///
/// \param image the \ref VkImage to transition
/// \param oldLayout the layout the image is currently in
/// \param newLayout the layout the image is transitioned to
void Texture2D::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer{ device->beginSingleTimeCommand() };

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = m_mipLevels,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    
    VkPipelineStageFlags srcStage{ 0 };
    VkPipelineStageFlags dstStage{ 0 };

    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    device->endSingleTimeCommand(commandBuffer);
}

} // namespace vv

