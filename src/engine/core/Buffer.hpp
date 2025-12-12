#ifndef VULKAN_VOXELS_SRC_ENGINE_CORE_BUFFER_HPP
#define VULKAN_VOXELS_SRC_ENGINE_CORE_BUFFER_HPP

#include "Device.hpp"

#include "vk_mem_alloc.h"
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <iterator>
#include <memory>
#include <type_traits>

namespace vv
{

namespace test
{

class BufferTestHelper;

}

/// \brief Abstraction over VkBuffer
///
/// Based on https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
///
/// \author Felix Hommel
/// \date 11/20/2025
class Buffer
{
    friend class test::BufferTestHelper;

public:
    /// \brief Create a new generic Buffer
    ///
    /// \param device the \ref Device where the buffer is created on
    /// \param elementSize how big a single element of data is (in byte)
    /// \param elementCount how many elements of data can fit in the buffer maximally
    /// \param usageFlags how the buffer is going to be used
    /// \param allocInfo help for vma to allocate the best possible memory type
    /// \param minOffsetAlignment (optional) the minimum required alignment for the offset member (in bytes)
    Buffer(
        std::shared_ptr<Device> device,
        VkDeviceSize elementSize,
        std::uint32_t elementCount,
        VkBufferUsageFlags usageFlags,
        const VmaAllocationCreateInfo& allocInfo,
        VkDeviceSize minOffsetAlignment = 1
    );
    ~Buffer();

    /// \brief Create a vertex buffer
    ///
    /// \param device the \ref Device where the buffer is created on
    /// \param elementSize how big a single element of data is (in byte)
    /// \param elementCount how many elements of data can fit in the buffer maximally
    ///
    /// \returns newly allocated \ref Buffer
    static Buffer createVertexBuffer(std::shared_ptr<Device> device, VkDeviceSize elementSize, std::uint32_t elementCount);
    /// \brief Create an index buffer
    ///
    /// \param device the \ref Device where the buffer is created on
    /// \param elementSize how big a single element of data is (in byte)
    /// \param elementCount how many elements of data can fit in the buffer maximally
    ///
    /// \returns newly allocated \ref Buffer
    static Buffer createIndexBuffer(std::shared_ptr<Device> device, VkDeviceSize elementSize, std::uint32_t elementCount);
    /// \brief Create a uniform buffer
    ///
    /// \note Writeable to from Host and automatically mapped on creation
    ///
    /// \param device the \ref Device where the buffer is created on
    /// \param elementSize how big a single element of data is (in byte)
    /// \param elementCount how many elements of data can fit in the buffer maximally
    ///
    /// \returns newly allocated \ref Buffer
    static Buffer createUniformBuffer(std::shared_ptr<Device> device, VkDeviceSize elementSize, std::uint32_t elementCount);
    /// \brief Create a storage buffer
    ///
    /// \param device the \ref Device where the buffer is created on
    /// \param elementSize how big a single element of data is (in byte)
    /// \param elementCount how many elements of data can fit in the buffer maximally
    ///
    /// \returns newly allocated \ref Buffer
    static Buffer createStorageBuffer(std::shared_ptr<Device> device, VkDeviceSize elementSize, std::uint32_t elementCount);
    /// \brief Create a staging buffer
    ///
    /// \note Writeable to from Host and automatically mapped on creation
    ///
    /// \param device the \ref Device where the buffer is created on
    /// \param elementSize how big a single element of data is (in byte)
    /// \param elementCount how many elements of data can fit in the buffer maximally
    ///
    /// \returns newly allocated \ref Buffer
    static Buffer createStagingBuffer(std::shared_ptr<Device> device, VkDeviceSize elementSize, std::uint32_t elementCount);

    Buffer(const Buffer&) = delete;
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&&) = delete;

    [[nodiscard]] VkBuffer getBuffer() const noexcept { return m_buffer; }
    [[nodiscard]] bool isCoherent() const noexcept { return m_isCoherent; }

    /// \brief Map the memory of the buffer so that it can be accessed by the CPU
    ///
    /// \returns the result of the vulkan mapping operation
    VkResult map();
    /// \brief Unmap the currently mapped memory range
    void unmap();

    /// \brief Write data to the buffer
    ///
    /// \tparam T can be any single data type (struct, built-in, ...) as long as it is trivially copyable and is no pointer
    /// \param data the data that is being written to the buffer
    /// \param offset (optional) offset into the buffer from where to begin writing memory (in byte)
    template <typename T>
        requires std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T> && (!std::is_pointer_v<T>)
    void writeToBuffer(const T& data, VkDeviceSize offset = 0)
    {
        writeToBufferRaw(&data, sizeof(data), offset);
    }
    /// \brief Write data to the buffer
    ///
    /// \tparam C can be any container that supports continuous iterators and whose data elements can be trivially copied
    /// \param data the data container that contains the data itself
    /// \param offset (optional) offset into the buffer from where to begin writing memory (in byte)
    template <typename C>
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
    void flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
    /// \brief Create a buffer info descriptor
    ///
    /// \param size (optional) size of the memory range of the descriptor (in byte)
    /// \param offset (optional) offset into the buffer (in byte)
    [[nodiscard]] VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
    /// \brief Invalidate a range of memory to make it available to the CPU
    ///
    /// \note Only required for non-coherent memory
    ///
    /// \param size (optional) size of the memory range (in byte)
    /// \param offset (optional) offset into the buffer (in byte)
    [[nodiscard]] VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

private:
    std::shared_ptr<Device> device;

    std::uint32_t m_elementCount; ///< How many elements of data are at maximum in the buffer
    VkDeviceSize m_elementSize;   ///< How big a single element is (in byte)
    VkDeviceSize m_alignmentSize; ///< How much space a single element needs with added alignment requirements (in byte)
    VkBufferUsageFlags m_usageFlags;
    VmaAllocationCreateInfo m_vmaAllocInfo;

    VkDeviceSize m_bufferSize;
    bool m_isCoherent;
    bool m_ownsMapping{ true };
    void* m_mapped{ nullptr };
    VkBuffer m_buffer{ VK_NULL_HANDLE };
    VmaAllocation m_allocation{ VK_NULL_HANDLE };

    static VkDeviceSize getAlignment(VkDeviceSize elementSize, VkDeviceSize minOffsetAlignment);

    void writeToBufferRaw(const void* pData, VkDeviceSize size, VkDeviceSize offset = 0) const;
};

} // namespace vv

#endif //! VULKAN_VOXELS_SRC_ENGINE_CORE_BUFFER_HPP
