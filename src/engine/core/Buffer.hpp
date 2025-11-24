#ifndef VULKAN_VOXELS_SRC_ENGINE_CORE_BUFFER_HPP
#define VULKAN_VOXELS_SRC_ENGINE_CORE_BUFFER_HPP

#include "Device.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <iterator>
#include <type_traits>

namespace vv
{

/// \brief Abstraction over VkBuffer
///
/// Based on https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
///
/// \author Felix Hommel
/// \date 11/20/2025
class Buffer
{
public:
    /// \brief Create a new Buffer
    ///
    /// \param device the \ref Device where the buffer is created on
    /// \param elementSize how big a single element of data is (in byte)
    /// \param elementCount how many elements of data can fit in the buffer maximally
    /// \param usageFlags how the buffer is going to be used
    /// \param memoryPropertyFlags which properties the memory needs to fulfill
    /// \param (optional) minOffsetAlignment the minimum required alignment for the offset member (in bytes)
    Buffer(
            Device& device,
            VkDeviceSize elementSize,
            std::uint32_t elementCount,
            VkBufferUsageFlags usageFlags,
            VkMemoryPropertyFlags memoryPropertyFlags,
            VkDeviceSize minOffsetAlignment = 1);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer(Buffer&&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&&) = delete;

    [[nodiscard]] VkBuffer getBuffer() const noexcept { return m_buffer; }

    /// \brief Map the memory of the buffer so that it can be accessed by the CPU
    ///
    /// \param size (optional) size of the range that is getting mapped
    /// \param offset (optional) offset into the buffer for the region that is getting mapped (in byte)
    ///
    /// \return the result of the vulkan mapping operation
    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    /// \brief Unmap the currently mapped memory range
    void unmap();

    /// \brief Write data to the buffer
    ///
    /// \param T can be any single data type (struct, built-in, ...) as long as it is trivially copyable and is no pointer
    /// \param data the data that is being written to the buffer
    /// \param offset (optional) offset into the buffer from where to begin writing memory (in byte)
    template<typename T>
        requires std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T> && (!std::is_pointer_v<T>)
    void writeToBuffer(const T& data, VkDeviceSize offset = 0)
    {
        writeToBufferRaw(&data, sizeof(data), offset);
    }
    /// \brief Write data to the buffer
    ///
    /// \param C can be any container that supports contignuous iterators and whose data elements can be trivially copied
    /// \param data the data container that contains the data itself
    /// \param offset (optional) offset into the buffer from where to begin writing memory (in byte)
    template<typename C>
        requires std::contiguous_iterator<typename C::pointer> && std::is_trivially_copyable_v<typename C::value_type>
    void writeToBuffer(const C& data, VkDeviceSize offset = 0)
    {
        using T = typename C::value_type;
        writeToBufferRaw(data.data(), sizeof(T) * data.size(), offset);
    }
    /// \brief Flush a range of memory to make it available to the GPU.
    ///
    /// \note Only required for non-coherent memory
    ///
    /// \param size (optional) size of the memory range to flush (in byte)
    /// \param offset (optional) offset from the beginning (in byte)
    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    /// \brief Create a buffer info descriptor 
    ///
    /// \param size (optional) size of the memory range of the descriptor (in byte)
    /// \param offset (optional) offset into the buffer (in byte)
    VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    /// \brief Invalidate a range of memory to make it available to the CPU
    ///
    /// \note Only required for non-coherent memory
    ///
    /// \param size (optional) size of the memory range (in byte)
    /// \param offset (optional) offset into the buffer (in byte)
    VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

private:
    Device& device;

    std::uint32_t m_elementCount; ///< How many elements of data are at maximum in the buffer
    VkDeviceSize m_elementSize; ///< How big a single element is (in byte)
    VkDeviceSize m_alignmentSize; ///< How much space a single element needs with added alignment requirements (in byte)
    VkBufferUsageFlags m_usageFlags;
    VkMemoryPropertyFlags m_memoryPropertyFlags;

    VkDeviceSize m_bufferSize;
    void* m_mapped{ nullptr };
    VkBuffer m_buffer{ VK_NULL_HANDLE };
    VkDeviceMemory m_bufferMemory{ VK_NULL_HANDLE };

    static VkDeviceSize getAlignment(VkDeviceSize elementSize, VkDeviceSize minOffsetAlignment);

    void writeToBufferRaw(const void* pData, VkDeviceSize size, VkDeviceSize offset = 0);
};

} //!vv

#endif //!VULKAN_VOXELS_SRC_ENGINE_CORE_BUFFER_HPP
