#ifndef VULKAN_VOXELS_SRC_ENGINE_CORE_DESCRIPTOR_SET_LAYOUT_HPP
#define VULKAN_VOXELS_SRC_ENGINE_CORE_DESCRIPTOR_SET_LAYOUT_HPP

#include "core/Device.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>

namespace vv
{

/// \brief A Descriptor set layout describes how a specific descriptor set looks like i.e., what are its data
///
/// \author Felix Hommel
/// \date 11/24/2025
class DescriptorSetLayout
{
public:
    /// \brief Builder to construct \ref DescriptorSetLayout
    ///
    /// \author Felix Hommel
    /// \date 11/24/2025
    class Builder
    {
    public:
        /// \brief Construct a new Builder
        ///
        /// \param device \ref Device where the Descriptor set layout is created on
        Builder(std::shared_ptr<Device> device) : device{ std::move(device) } {}

        /// \brief Add a binding to the descriptor set layout
        ///
        /// \param bindinig the binding location
        /// \param descriptorType what type of descriptor this binding is
        /// \param stageFlags which shader stage the descriptor layout is available in
        /// \param count (optional)
        Builder& addBinding(
            std::uint32_t binding,
            VkDescriptorType descriptorType,
            VkShaderStageFlags stageFlags,
            std::uint32_t count = 1
        );
        /// \brief build the \ref DescriptorSetLayout
        ///
        /// \returns \ref DescriptorSetLayout wrapped in a unique_ptr
        [[nodiscard]] std::unique_ptr<DescriptorSetLayout> build() const;
        /// \brief build the \ref DescriptorSetLayout
        ///
        /// \returns \ref DescriptorSetLayout wrapped in a shared_ptr
        [[nodiscard]] std::shared_ptr<DescriptorSetLayout> buildShared() const;

    private:
        std::shared_ptr<Device> device;
        std::unordered_map<std::uint32_t, VkDescriptorSetLayoutBinding> m_bindings;
    };

    /// \brief Construct a new DescriptorSetLayout
    ///
    /// \param device \ref Device where the Descriptor set layout is created on
    /// \param bindings a map containing the layout bindings
    DescriptorSetLayout(
        std::shared_ptr<Device> device, std::unordered_map<std::uint32_t, VkDescriptorSetLayoutBinding> bindings
    );
    ~DescriptorSetLayout();

    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout(DescriptorSetLayout&&) = delete;
    DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

    [[nodiscard]] VkDescriptorSetLayout getDescriptorLayout() const noexcept { return m_descriptorSetLayout; }

private:
    std::shared_ptr<Device> m_device;
    std::unordered_map<std::uint32_t, VkDescriptorSetLayoutBinding> m_bindings;
    VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };

    friend class DescriptorWriter;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_CORE_DESCRIPTOR_SET_LAYOUT_HPP
