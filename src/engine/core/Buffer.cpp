#include "Buffer.hpp"

#include "core/Device.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>

namespace vv
{

Buffer::Buffer(
            Device& device,
            VkDeviceSize elementSize,
            std::uint32_t elementCount,
            VkBufferUsageFlags usageFlags,
            VkMemoryPropertyFlags memoryPropertyFlags,
            VkDeviceSize minOffsetAlignment)
    : device(device)
    , m_elementCount{ elementCount }
    , m_elementSize{ elementSize }
    , m_alignmentSize{ getAlignment(m_elementSize, minOffsetAlignment) }
    , m_usageFlags{ usageFlags }
    , m_memoryPropertyFlags{ memoryPropertyFlags }
    , m_bufferSize{ m_alignmentSize * m_elementCount }
{
    device.createBuffer(m_bufferSize, m_usageFlags, m_memoryPropertyFlags, m_buffer, m_bufferMemory);
}

Buffer::~Buffer()
{
    unmap();
    vkDestroyBuffer(device.device(), m_buffer, nullptr);
    vkFreeMemory(device.device(), m_bufferMemory, nullptr);
}

VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset)
{
#if defined(VV_ENABLE_ASSERTS)
    assert(m_buffer != VK_NULL_HANDLE && m_bufferMemory != VK_NULL_HANDLE && "map() called before buffer is craeted");
#endif

    return vkMapMemory(device.device(), m_bufferMemory, offset, size, 0, &m_mapped);
}

void Buffer::unmap()
{
    if(m_mapped == nullptr)
        return;

    vkUnmapMemory(device.device(), m_bufferMemory);
    m_mapped = nullptr;

}

VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange{};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_bufferMemory;
    mappedRange.offset = offset;
    mappedRange.size = size;

    return vkFlushMappedMemoryRanges(device.device(), 1, &mappedRange);
}

VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset)
{
    return VkDescriptorBufferInfo{
        .buffer = m_buffer,
        .offset = offset,
        .range = size
    };
}

VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange{};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = m_bufferMemory;
    mappedRange.offset = offset;
    mappedRange.size = size;

    return vkInvalidateMappedMemoryRanges(device.device(), 1, &mappedRange);
}

/// \brief Determine the minimum sice that a element needs to be stored in the buffer
///
/// To fulfill alignment requirements the raw size of the element might not be suitable and therefore an extra
/// amount of padding needs to be added
///
/// \param elementSize the size of an raw element (in byte)
/// \param minOffsetAlignment the minimum required alignment for the offset member (in bytes)
///
/// \returns the determined size of an element in the buffer including alignment
VkDeviceSize Buffer::getAlignment(VkDeviceSize elementSize, VkDeviceSize minOffsetAlignment)
{
    if(minOffsetAlignment > 0)
        return (elementSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);

    return elementSize;
}

/// \brief Write data to the buffer
///
/// \param pData pointer to the data in CPU accessible memory
/// \param size of the data pointed to by pData (in bytes)
/// \param offset (optional) offset in to the buffer from where to start writing (in bytes)
void Buffer::writeToBufferRaw(const void* pData, VkDeviceSize size, VkDeviceSize offset)
{
#if defined(VV_ENABLE_ASSERTS)
    assert(m_mapped  != nullptr && "Cannot copy to unmapped buffer");
    assert(offset + size <= m_bufferSize && "The buffer that is being written to the buffer execeeds the buffer's size");
#endif

    if(offset == 0) [[likely]] // NOTE: unless in the future more buffers are using offsets this may be a small optimization
        std::memcpy(m_mapped, pData, size);
    else
    {
        auto* memOffset{ std::next(static_cast<std::byte*>(m_mapped), static_cast<std::ptrdiff_t>(offset)) };

        std::memcpy(memOffset, pData, size);
    }
}

} // !vv
