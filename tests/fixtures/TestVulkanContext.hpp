#ifndef VULKAN_VOXELS_TESTS_TEST_VULKAN_CONTEXT_HPP
#define VULKAN_VOXELS_TESTS_TEST_VULKAN_CONTEXT_HPP

#include "core/Device.hpp"

#include <vulkan/vulkan_core.h>

#include <memory>

namespace vv::test
{

class TestVulkanContext
{
public:
    TestVulkanContext() = default;
	~TestVulkanContext() { vkDeviceWaitIdle(m_device->device()); }

	TestVulkanContext(const TestVulkanContext&) = default;
	TestVulkanContext(TestVulkanContext&&) = delete;
	TestVulkanContext& operator=(const TestVulkanContext&) = default;
	TestVulkanContext& operator=(TestVulkanContext&&) = delete;

    [[nodiscard]] std::shared_ptr<Device> device() const noexcept { return m_device; }

private:
    std::shared_ptr<Device> m_device{ std::make_shared<Device>(true) };
};

} // namespace vv::test

#endif // !VULKAN_VOXELS_TESTS_TEST_VULKAN_CONTEXT_HPP
