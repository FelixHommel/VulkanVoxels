#ifndef VULKAN_VOXELS_SRC_ENGINE_CORE_DESCRIPTOR_WRITER_HPP
#define VULKAN_VOXELS_SRC_ENGINE_CORE_DESCRIPTOR_WRITER_HPP

#include "core/DescriptorPool.hpp"
#include "core/DescriptorSetLayout.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <vector>

namespace vv
{

/// \brief The DescriptorWriter handles the actual allocation process by coordinating all the required steps
///
/// Users of the DescriptorWriter just need to provide the respecting Info struct and a binding number and then
/// based on that the DescriptorWriter deals with the rest to allocate the descriptor.
///
/// \author Felix Hommel
/// \date 11/24/2025
class DescriptorWriter
{
public:
    /// \brief Construct a new \ref DescriptorWriter
    ///
    /// \param setLayout a \ref DescriptorSetLayout describes the layout of the descriptor
    /// \param pool the \ref DescriptorPool where the descriptor sets are allocated on
    DescriptorWriter(DescriptorSetLayout* setLayout, DescriptorPool* pool);
    ~DescriptorWriter() = default;

    DescriptorWriter(const DescriptorWriter&) = default;
    DescriptorWriter(DescriptorWriter&&) = delete;
    DescriptorWriter& operator=(const DescriptorWriter&) = delete;
    DescriptorWriter& operator=(DescriptorWriter&&) = delete;

    /// \brief add a new buffer descriptor
    ///
    /// \param binding the binding number where the buffer is accessible
    /// \param bufferInfo the VkDescriptorBufferInfo struct that contains the descriptors details
    DescriptorWriter& writeBuffer(std::uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
    /// \brief add a new image descriptor
    ///
    /// \param binding the binding number where the buffer is accessible
    /// \param imageInfo the VkDescriptorImageInfo struct that contains the descriptors details
    DescriptorWriter& writeImage(std::uint32_t binding, VkDescriptorImageInfo* imageInfo);

    /// \brief build the descriptor set
    ///
    /// uses overwrite() internally
    ///
    /// \param set reference to the VkDescriptorSet that contains the data
    bool build(VkDescriptorSet& set);
    /// \brief update the descriptor 
    ///
    /// \param set the descriptor set which will be updated
    void overwrite(VkDescriptorSet& set);

private:
    DescriptorSetLayout* setLayout;
    DescriptorPool* pool;
    std::vector<VkWriteDescriptorSet> m_writes;
};

} // !vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_CORE_DESCRIPTOR_WRITER_HPP
