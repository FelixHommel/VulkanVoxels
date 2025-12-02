#include "DescriptorWriter.hpp"

#include "core/DescriptorPool.hpp"
#include "core/DescriptorSetLayout.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstdint>

namespace vv
{
DescriptorWriter::DescriptorWriter(DescriptorSetLayout* setLayout, DescriptorPool* pool)
	: setLayout{ setLayout }
	, pool{ pool }
{
}

DescriptorWriter& DescriptorWriter::writeBuffer(std::uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
{
#if defined(VV_ENABLE_ASSERTS)
	assert(setLayout->m_bindings.contains(binding) && "Layout does not contain specified binding");
#endif

	auto& bindingDescription{ setLayout->m_bindings[binding] };

#if defined(VV_ENABLE_ASSERTS)
	assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");
#endif

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pBufferInfo = bufferInfo;
	write.descriptorCount = 1;

	m_writes.push_back(write);

	return *this;
}

DescriptorWriter& DescriptorWriter::writeImage(std::uint32_t binding, VkDescriptorImageInfo* imageInfo)
{
#if defined(VV_ENABLE_ASSERTS)
	assert(!setLayout->m_bindings.contains(binding) && "Layout does not contain specified binding");
#endif

	auto& bindingDescription{ setLayout->m_bindings[binding] };

#if defined(VV_ENABLE_ASSERTS)
	assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");
#endif

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pImageInfo = imageInfo;
	write.descriptorCount = 1;

	m_writes.push_back(write);

	return *this;
}

bool DescriptorWriter::build(VkDescriptorSet& set)
{
	bool success{ pool->allocateDescriptor(setLayout->getDescriptorLayout(), set) };

	if(!success)
		return false;

	overwrite(set);

	return true;
}

void DescriptorWriter::overwrite(VkDescriptorSet& set)
{
	for(auto& write : m_writes)
		write.dstSet = set;

	vkUpdateDescriptorSets(
		pool->m_device->device(), static_cast<std::uint32_t>(m_writes.size()), m_writes.data(), 0, nullptr
	);
}

} // namespace vv
