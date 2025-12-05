#include "core/Buffer.hpp"
#include "fixtures/TestVulkanContext.hpp"
#include "helper/BufferTestHelper.hpp"

#include "gtest/gtest.h"
#include <vulkan/vulkan_core.h>

#include <memory>
#include <ranges>
#include <span>
#include <vector>

namespace vv::test
{


class BufferTest : public ::testing::Test
{
protected:
    static constexpr auto ELEMENT_SIZE{ sizeof(float) };
    static constexpr auto ALLOCATIONS{ 100u };
    static constexpr auto BUFFER_USAGE{ VK_BUFFER_USAGE_VERTEX_BUFFER_BIT };
    static constexpr auto BUFFER_PROPERTIES{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT };

    void SetUp() override
    {
        ctx = std::make_unique<TestVulkanContext>();
    }

    std::unique_ptr<TestVulkanContext> ctx;

};

TEST_F(BufferTest, AllocateBuffer)
{
    Buffer buffer(
        ctx->device(),
        ELEMENT_SIZE,
        ALLOCATIONS,
        BUFFER_USAGE,
        BUFFER_PROPERTIES
    );

    EXPECT_NE(buffer.getBuffer(), VK_NULL_HANDLE);
}

TEST_F(BufferTest, MemoryMappingOperations)
{
    Buffer buffer(
        ctx->device(),
        ELEMENT_SIZE,
        ALLOCATIONS,
        BUFFER_USAGE,
        BUFFER_PROPERTIES
    );

    EXPECT_EQ(buffer.map(), VK_SUCCESS);
    EXPECT_NE(BufferTestHelper::getMappedMemory(buffer), nullptr);

    buffer.unmap();
    EXPECT_EQ(BufferTestHelper::getMappedMemory(buffer), nullptr);
}

TEST_F(BufferTest, WriteToBuffer)
{
    Buffer buffer(
        ctx->device(),
        ELEMENT_SIZE,
        ALLOCATIONS,
        BUFFER_USAGE,
        BUFFER_PROPERTIES
    );

    buffer.map();
    const std::vector<float> writeData{ 1.f, 2.f, 3.f, 4.f, 5.f };
    buffer.writeToBuffer(writeData);
    EXPECT_EQ(buffer.flush(), VK_SUCCESS);
    buffer.unmap();

    buffer.map();
    const float* rawDataRead{ static_cast<const float*>(BufferTestHelper::getMappedMemory(buffer)) };
    const std::span<const float> readData(rawDataRead, writeData.size());
    for(const auto& [readValue, writeValue] : std::views::zip(readData, writeData))
        EXPECT_FLOAT_EQ(readValue, writeValue);
    buffer.unmap();
}

} // namespace vv::test
