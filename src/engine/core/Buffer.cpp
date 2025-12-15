#include "Buffer.hpp"

#include "core/Device.hpp"

#include "vk_mem_alloc.h"
#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <utility>

namespace vv
{

Buffer Buffer::createVertexBuffer(std::shared_ptr<Device> device, VkDeviceSize elementSize, std::uint32_t elementCount)
{
    VkBufferUsageFlags usage{ VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT };
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    return { std::move(device), elementSize, elementCount, usage, allocInfo };
}

Buffer Buffer::createIndexBuffer(std::shared_ptr<Device> device, VkDeviceSize elementSize, std::uint32_t elementCount)
{
    VkBufferUsageFlags usage{ VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT };
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    return { std::move(device), elementSize, elementCount, usage, allocInfo };
}

Buffer Buffer::createUniformBuffer(std::shared_ptr<Device> device, VkDeviceSize elementSize, std::uint32_t elementCount)
{
    auto offset{ device->properties.limits.minUniformBufferOffsetAlignment };
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    return { std::move(device), elementSize, elementCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, allocInfo, offset };
}

Buffer Buffer::createStorageBuffer(std::shared_ptr<Device> device, VkDeviceSize elementSize, std::uint32_t elementCount)
{
    VkBufferUsageFlags usage{ VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT };
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    return { std::move(device), elementSize, elementCount, usage, allocInfo };
}

Buffer Buffer::createStagingBuffer(std::shared_ptr<Device> device, VkDeviceSize elementSize, std::uint32_t elementCount)
{
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    return { std::move(device), elementSize, elementCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, allocInfo };
}

Buffer Buffer::createImageStagingBuffer(std::shared_ptr<Device> device, VkDeviceSize elementSize, std::uint32_t elementCount)
{
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    // NOTE: For vkCmdCopyBufferToImage the minAlignment needs to be a multiple of 4
    return { std::move(device), elementSize, elementCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, allocInfo, 4 };
}

Buffer::Buffer(
    std::shared_ptr<Device> pDevice,
    VkDeviceSize elementSize,
    std::uint32_t elementCount,
    VkBufferUsageFlags usageFlags,
    const VmaAllocationCreateInfo& allocInfo,
    VkDeviceSize minOffsetAlignment
)
    : device(std::move(pDevice))
    , m_elementCount{ elementCount }
    , m_elementSize{ elementSize }
    , m_alignmentSize{ getAlignment(m_elementSize, minOffsetAlignment) }
    , m_usageFlags{ usageFlags }
    , m_vmaAllocInfo{ allocInfo }
    , m_bufferSize{ m_alignmentSize * m_elementCount }
{
    device->createBuffer(m_bufferSize, m_usageFlags, allocInfo, m_buffer, m_allocation);

    VmaAllocationInfo info{};
    vmaGetAllocationInfo(device->allocator(), m_allocation, &info);
    // NOLINTNEXTLINE(*-pro-bounds-constant-array-index): Vulkan API requires accessing C-style arrays by index
    const VkMemoryType& type{ device->memoryProperties.memoryTypes[info.memoryType] };

    m_isCoherent = ((type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0);
    if(info.pMappedData != nullptr)
    {
        m_ownsMapping = false;
        m_mapped = info.pMappedData;
    }
}

Buffer::~Buffer()
{
    unmap();
    vmaDestroyBuffer(device->allocator(), m_buffer, m_allocation);
}

Buffer::Buffer(Buffer&& other) noexcept
    : device(other.device)
    , m_elementCount(other.m_elementCount)
    , m_elementSize(other.m_elementSize)
    , m_alignmentSize(other.m_alignmentSize)
    , m_usageFlags(other.m_usageFlags)
    , m_vmaAllocInfo(other.m_vmaAllocInfo)
    , m_bufferSize(other.m_bufferSize)
    , m_isCoherent(other.m_isCoherent)
    , m_ownsMapping(other.m_ownsMapping)
    , m_mapped(other.m_mapped)
    , m_buffer(other.m_buffer)
    , m_allocation(other.m_allocation)
{
    other.m_buffer = VK_NULL_HANDLE;
    other.m_allocation = VK_NULL_HANDLE;
    other.m_mapped = nullptr;
    other.m_bufferSize = 0;
}

VkResult Buffer::map()
{
#if defined(VV_ENABLE_ASSERTS)
    assert(m_buffer != VK_NULL_HANDLE && m_allocation != VK_NULL_HANDLE && "map() called before buffer is created");
#endif

    m_ownsMapping = true;
    return vmaMapMemory(device->allocator(), m_allocation, &m_mapped);
}

void Buffer::unmap()
{
    if(m_mapped == nullptr && m_ownsMapping)
        return;

    if(!m_ownsMapping)
    {
        m_mapped = nullptr;
        return;
    }

    vmaUnmapMemory(device->allocator(), m_allocation);
    m_mapped = nullptr;
}

void Buffer::writeToBufferRaw(const void* pData, VkDeviceSize size, VkDeviceSize offset) const
{
#if defined(VV_ENABLE_ASSERTS)
    assert(m_mapped != nullptr && "Cannot copy to unmapped buffer");
    assert(offset + size <= m_bufferSize && "The data that is being written to the buffer exceeds the buffer's size");
#endif

    if(offset == 0)
        [[likely]] // NOTE: unless in the future more buffers are using offsets this may be a small optimization
        std::memcpy(m_mapped, pData, size);
    else
    {
        auto* memOffset{ std::next(static_cast<std::byte*>(m_mapped), static_cast<std::ptrdiff_t>(offset)) };

        std::memcpy(memOffset, pData, size);
    }
}

void Buffer::flush(VkDeviceSize size, VkDeviceSize offset) const
{
    if(m_isCoherent)
        return;

    vmaFlushAllocation(device->allocator(), m_allocation, offset, size);
}

VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) const
{
    return VkDescriptorBufferInfo{ .buffer = m_buffer, .offset = offset, .range = size };
}

VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) const
{
    return vmaInvalidateAllocation(device->allocator(), m_allocation, offset, size);
}

/// \brief Determine the minimum sice that a element needs to be stored in the buffer
///
/// To fulfill alignment requirements the raw size of the element might not be suitable and therefore an extra
/// amount of padding needs to be added
///
/// \param elementSize the size of a raw element (in byte)
/// \param minOffsetAlignment the minimum required alignment for the offset member (in bytes)
///
/// \returns the determined size of an element in the buffer including alignment
VkDeviceSize Buffer::getAlignment(VkDeviceSize elementSize, VkDeviceSize minOffsetAlignment)
{
    if(minOffsetAlignment > 0)
        return (elementSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);

    return elementSize;
}

} // namespace vv
