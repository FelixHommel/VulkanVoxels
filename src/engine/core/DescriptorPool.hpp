#ifndef VULKAN_VOXELS_SRC_ENGINE_CORE_DESCRIPTOR_POOL_HPP
#define VULKAN_VOXELS_SRC_ENGINE_CORE_DESCRIPTOR_POOL_HPP

#include "core/Device.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace vv
{

/// \brief Descriptor pools are used to allocate different kind of descriptors (i.e., Buffer, Image, ...)
///
/// \author Felix Hommel
/// \date 11/24/2025
class DescriptorPool
{
public:
    /// \brief Builder for a \ref DescriptorPool
    ///
    /// \author Felix Hommel
    /// \date 11/24/2025
    class Builder
    {
    public:
        /// \brief Construct a new Descriptor pool builder
        ///
        /// \param device the \ref Devuce on which the descripto pool is created
        Builder(Device& device) : device(device) {}

        /// \brief add more descriptor types to the pool
        ///
        /// \param descriptorType for which type of descriptor pool size is being added
        /// \param count how many descriptors of the type are added
        Builder& addPoolSize(VkDescriptorType descriptorType, std::uint32_t count);
        /// \brief configure the flags that are used to create the descriptor pool
        /// 
        /// \param createFlags configuration flags
        Builder& setPoolFlags(VkDescriptorPoolCreateFlags createFlags);
        /// \brief Configure the maximum amount of descriptors that can be allocated from the descriptor pool
        ///
        /// \param count maximum amount of possible allocations
        Builder& setMaxSets(std::uint32_t count);
        /// \brief build the descriptor pool based on the configuration
        ///
        /// \returns \ref DescriptorPool wrapped in a unique_ptr
        [[nodiscard]] std::unique_ptr<DescriptorPool> build() const;

    private:
        static constexpr std::uint32_t DEFAULT_MAX_SETS{ 1000 };

        Device& device; // NOLINT
        std::vector<VkDescriptorPoolSize> m_poolSizes;
        std::uint32_t m_maxSets{ DEFAULT_MAX_SETS };
        VkDescriptorPoolCreateFlags m_createFlags{ 0 };
    };

    /// \brief Construct a new DescriptorPool
    ///
    /// \param device the device where the descriptor pool is created on
    /// \param maxSize how many allocations the descriptor pool can do at a time
    /// \param createFlags configuration flags for construction
    /// \param poolSizes describes how many descriptors of a certain type are to allocate
    DescriptorPool(
        Device& device,
        std::uint32_t maxSets,
        VkDescriptorPoolCreateFlags createFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes
    );
    ~DescriptorPool();

    DescriptorPool(const DescriptorPool&) = delete;
    DescriptorPool(DescriptorPool&&) = delete;
    DescriptorPool& operator=(const DescriptorPool&) = delete;
    DescriptorPool& operator=(DescriptorPool&&) = delete;

    /// \brief Allocate a new descriptor
    ///
    /// \param descriptorLayout the layout of the descriptor
    /// \param reference to the descriptor that is being allocated
    ///
    /// \returns *true* if the allocation was successfull, *false* otherwise
    bool allocateDescriptor(VkDescriptorSetLayout descriptorLayout, VkDescriptorSet& descriptor) const;
    /// \brief Free the descriptor pool from descriptors that were allocated
    ///
    /// \param descriptors the descriptors that are being freed
    void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;
    /// \brief reset the entire descriptor pool
    ///
    /// This operation frees every allocated descriptor
    void resetPool();

private:
    Device& device;
    VkDescriptorPool m_descriptorPool{ VK_NULL_HANDLE };

    friend class DescriptorWriter;
};

} // !vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_CORE_DESCRIPTOR_POOL_HPP
