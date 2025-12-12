#include "core/Buffer.hpp"
#include "core/Device.hpp"
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
    static constexpr auto VMA_ALLOC{
        VmaAllocationCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            .requiredFlags = 0,
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.f
        }
    };

    void SetUp() override
    {
        ctx = std::make_unique<TestVulkanContext>();
    }

    std::unique_ptr<TestVulkanContext> ctx;

};

TEST_F(BufferTest, AllocateBufferWithConstrutor)
{
    const Buffer buffer(
        ctx->device(),
        ELEMENT_SIZE,
        ALLOCATIONS,
        BUFFER_USAGE,
        VMA_ALLOC
    );

    EXPECT_NE(buffer.getBuffer(), VK_NULL_HANDLE);
}

TEST_F(BufferTest, UniformBufferIsHostCoherent)
{
    const Buffer uBuffer{ Buffer::createUniformBuffer(ctx->device(), ELEMENT_SIZE, ALLOCATIONS) };
    EXPECT_TRUE(uBuffer.isCoherent());
}

TEST_F(BufferTest, MemoryMappingOperations)
{
    Buffer buffer(
        ctx->device(),
        ELEMENT_SIZE,
        ALLOCATIONS,
        BUFFER_USAGE,
        VMA_ALLOC
    );

    ASSERT_EQ(buffer.map(), VK_SUCCESS);
    EXPECT_NE(BufferTestHelper::getMappedMemory(buffer), nullptr);

    buffer.unmap();
    EXPECT_EQ(BufferTestHelper::getMappedMemory(buffer), nullptr);
    EXPECT_NO_FATAL_FAILURE({ buffer.unmap(); }) << "Unmapping memory that already was unmapped should be save";
}

TEST_F(BufferTest, WriteToBuffer)
{
    Buffer buffer(
        ctx->device(),
        ELEMENT_SIZE,
        ALLOCATIONS,
        BUFFER_USAGE,
        VMA_ALLOC
    );

    buffer.map();
    const std::vector<float> writeData{ 1.f, 2.f, 3.f, 4.f, 5.f };
    buffer.writeToBuffer(writeData);
    ASSERT_NO_FATAL_FAILURE({ buffer.flush(); });
    buffer.unmap();

    buffer.map();
    const float* rawDataRead{ static_cast<const float*>(BufferTestHelper::getMappedMemory(buffer)) };
    const std::span<const float> readData(rawDataRead, writeData.size());
    for(const auto& [readValue, writeValue] : std::views::zip(readData, writeData))
        EXPECT_FLOAT_EQ(readValue, writeValue);
    buffer.unmap();
}

TEST_F(BufferTest, BufferMoveConstructor)
{
    std::unique_ptr<Buffer> buffer;
    ASSERT_NO_FATAL_FAILURE({
        buffer = std::make_unique<Buffer>(Buffer::createVertexBuffer(ctx->device(), ELEMENT_SIZE, ALLOCATIONS));
    });

    EXPECT_NE(buffer->getBuffer(), nullptr);
}

TEST_F(BufferTest, CreateBufferMapped)
{
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    Buffer buffer(
        ctx->device(),
        ELEMENT_SIZE,
        ALLOCATIONS,
        BUFFER_USAGE,
        allocInfo
    );

    EXPECT_NE(buffer.getBuffer(), VK_NULL_HANDLE);
    EXPECT_NE(BufferTestHelper::getMappedMemory(buffer), nullptr);
    ASSERT_NO_FATAL_FAILURE({ buffer.unmap(); });
}

TEST_F(BufferTest, CreateVertexBuffer)
{
    const auto buffer{ Buffer::createVertexBuffer(ctx->device(), ELEMENT_SIZE, ALLOCATIONS) };

    EXPECT_NE(buffer.getBuffer(), VK_NULL_HANDLE);
    EXPECT_EQ(BufferTestHelper::getMappedMemory(buffer), nullptr);
}

TEST_F(BufferTest, CreateIndexBuffer)
{
    const auto buffer{ Buffer::createIndexBuffer(ctx->device(), ELEMENT_SIZE, ALLOCATIONS) };

    EXPECT_NE(buffer.getBuffer(), VK_NULL_HANDLE);
    EXPECT_EQ(BufferTestHelper::getMappedMemory(buffer), nullptr);
}

TEST_F(BufferTest, CreateUniformBuffer)
{
    const auto buffer{ Buffer::createUniformBuffer(ctx->device(), ELEMENT_SIZE, ALLOCATIONS) };

    EXPECT_NE(buffer.getBuffer(), VK_NULL_HANDLE);
    EXPECT_NE(BufferTestHelper::getMappedMemory(buffer), nullptr);
}

TEST_F(BufferTest, CreateStorageBuffer)
{
    const auto buffer{ Buffer::createStorageBuffer(ctx->device(), ELEMENT_SIZE, ALLOCATIONS) };

    EXPECT_NE(buffer.getBuffer(), VK_NULL_HANDLE);
    EXPECT_EQ(BufferTestHelper::getMappedMemory(buffer), nullptr);
}

TEST_F(BufferTest, CreateStagingBuffer)
{
    const auto buffer{ Buffer::createStagingBuffer(ctx->device(), ELEMENT_SIZE, ALLOCATIONS) };

    EXPECT_NE(buffer.getBuffer(), VK_NULL_HANDLE);
    EXPECT_NE(BufferTestHelper::getMappedMemory(buffer), nullptr);
}

} // namespace vv::test
