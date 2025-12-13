#include "DescriptorSetLayout.hpp"

#include "core/Device.hpp"
#include "utility/exceptions/VulkanException.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace vv
{

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(
    std::uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, std::uint32_t count
)
{
#if defined(VV_ENABLE_ASSERTS)
    assert(!m_bindings.contains(binding) && "Binding is already in use");
#endif

    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;

    m_bindings[binding] = layoutBinding;

    return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const
{
    return std::make_unique<DescriptorSetLayout>(device, m_bindings);
}

DescriptorSetLayout::DescriptorSetLayout(
    std::shared_ptr<Device> device, std::unordered_map<std::uint32_t, VkDescriptorSetLayoutBinding> bindings
)
    : m_device{ std::move(device) }, m_bindings{ bindings }
{
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings{};
    for(auto [_, b] : m_bindings)
        layoutBindings.push_back(b);

    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = static_cast<std::uint32_t>(layoutBindings.size());
    createInfo.pBindings = layoutBindings.data();

    const VkResult result{
        vkCreateDescriptorSetLayout(m_device->device(), &createInfo, nullptr, &m_descriptorSetLayout)
    };
    if(result != VK_SUCCESS)
        throw VulkanException("failed to create descriptor set layout", result);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(m_device->device(), m_descriptorSetLayout, nullptr);
}

} // namespace vv
