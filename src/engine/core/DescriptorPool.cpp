#include "DescriptorPool.hpp"

#include "spdlog/spdlog.h"

#include <iostream>
#include <vulkan/vulkan_core.h>

#include <memory>
#include <stdexcept>

namespace vv
{

DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, std::uint32_t count)
{
    m_poolSizes.push_back({ .type = descriptorType, .descriptorCount = count });

    return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags createFlags)
{
    m_createFlags = createFlags;

    return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(std::uint32_t count)
{
    m_maxSets = count;

    return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const
{
    return std::make_unique<DescriptorPool>(m_device, m_maxSets, m_createFlags, m_poolSizes);
}

DescriptorPool::DescriptorPool(
        std::shared_ptr<Device> device,
        std::uint32_t maxSets,
        VkDescriptorPoolCreateFlags createFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes
)
    : m_device{ device }
{
    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.poolSizeCount = static_cast<std::uint32_t>(poolSizes.size());
    createInfo.pPoolSizes = poolSizes.data();
    createInfo.maxSets = maxSets;
    createInfo.flags = createFlags;

    if(vkCreateDescriptorPool(device->device(), &createInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor pool");
}

DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(m_device->device(), m_descriptorPool, nullptr);
}

bool DescriptorPool::allocateDescriptor(VkDescriptorSetLayout descriptorLayout, VkDescriptorSet& descriptor) const
{
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorLayout;

    // TODO: Implement a more robust system to handle the case that the pool is full. https://vkguide.dev/docs/new_chapter_4/descriptor_abstractions/
    return vkAllocateDescriptorSets(m_device->device(), &allocInfo, &descriptor) == VK_SUCCESS;
}

void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
{
    vkFreeDescriptorSets(m_device->device(), m_descriptorPool, static_cast<std::uint32_t>(descriptors.size()), descriptors.data());
}

void DescriptorPool::resetPool()
{
    vkResetDescriptorPool(m_device->device(), m_descriptorPool, 0);
}

} // !vv
