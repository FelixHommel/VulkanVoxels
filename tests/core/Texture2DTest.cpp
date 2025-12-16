#include "fixtures/TestVulkanContext.hpp"
#include "helper/RandomNumberGenerator.hpp"

#include "core/Texture2D.hpp"
#include "utility/exceptions/FileException.hpp"

#include "gtest/gtest.h"
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

namespace vv::test
{

class Texture2DTest : public ::testing::Test
{
public:
    static constexpr auto TEST_TEXTURE_PATH{ PROJECT_ROOT "tests/resources/512x512.png" };
    void SetUp() override { ctx = std::make_unique<TestVulkanContext>(); }

    std::unique_ptr<TestVulkanContext> ctx;
};

TEST_F(Texture2DTest, createTextureFromMemory)
{
    // NOTE: 256x256 image, completely white
    constexpr std::uint32_t w{ 256 };
    constexpr std::uint32_t h{ 256 };
    const std::vector<std::uint8_t> pixels(static_cast<std::size_t>(w * h * 4), 255);
    const auto tex{ Texture2D(ctx->device(), w, h, TextureConfig::albedo(), std::as_bytes(std::span(pixels))) };

    EXPECT_NE(tex.image(), VK_NULL_HANDLE);
}

TEST_F(Texture2DTest, MoveConstructor)
{
    auto tex1{ Texture2D::loadFromFile(ctx->device(), TEST_TEXTURE_PATH, TextureConfig::albedo()) };
    VkImage original{ tex1.image() };

    const auto tex2{ std::move(tex1) };

    EXPECT_EQ(original, tex2.image());
    EXPECT_EQ(tex1.image(), VK_NULL_HANDLE);
}

TEST_F(Texture2DTest, CorrectDimensions)
{
    constexpr std::uint32_t dim{ 512 };
    const auto tex{ Texture2D::loadFromFile(ctx->device(), TEST_TEXTURE_PATH, TextureConfig::albedo()) };

    EXPECT_EQ(tex.width(), dim);
    EXPECT_EQ(tex.height(), dim);
}

TEST_F(Texture2DTest, SRGBFormat)
{
    const auto tex{ Texture2D::loadFromFile(ctx->device(), TEST_TEXTURE_PATH, TextureConfig::albedo()) };

    EXPECT_EQ(tex.format(), VK_FORMAT_R8G8B8A8_SRGB);
}

TEST_F(Texture2DTest, LinearFormat)
{
    const auto tex{ Texture2D::loadFromFile(ctx->device(), TEST_TEXTURE_PATH, TextureConfig::normal()) };

    EXPECT_EQ(tex.format(), VK_FORMAT_R8G8B8A8_UNORM);
}

TEST_F(Texture2DTest, ValidDescriporInfo)
{
    const auto tex{ Texture2D::loadFromFile(ctx->device(), TEST_TEXTURE_PATH, TextureConfig::albedo()) };
    const auto desc{ tex.descriptor() };

    EXPECT_NE(desc.sampler, VK_NULL_HANDLE);
    EXPECT_NE(desc.imageView, VK_NULL_HANDLE);
    EXPECT_EQ(desc.imageLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

TEST_F(Texture2DTest, MipmapGeneration)
{
    // NOTE: log2(512) + 1 = 10 mip levels
    const std::uint32_t mipLevels{ static_cast<std::uint32_t>(std::floor(std::log2(512))) + 1 };
    const auto tex{ Texture2D::loadFromFile(ctx->device(), TEST_TEXTURE_PATH, TextureConfig::albedo()) };

    EXPECT_EQ(tex.mipLevels(), mipLevels);
}

TEST_F(Texture2DTest, InvalidFilePath)
{
    ASSERT_THROW(Texture2D::loadFromFile(ctx->device(), "nonexistent.png", TextureConfig::albedo()), FileException);
}

TEST_F(Texture2DTest, RandomDimensions)
{
    constexpr std::uint32_t maxSize{ 1025 };
    constexpr unsigned int testCases{ 100 };
    for(unsigned int i{ 0 }; i < testCases; ++i)
    {
        const std::uint32_t w{ generateRandom<std::uint32_t>(1, maxSize) };
        const std::uint32_t h{ generateRandom<std::uint32_t>(1, maxSize) };
        const std::vector<std::uint8_t> pixels(static_cast<std::size_t>(w * h) * 4, 0);
        const Texture2D tex{ ctx->device(), w, h, TextureConfig::albedo(), std::as_bytes(std::span(pixels)) };

        EXPECT_EQ(tex.width(), w);
        EXPECT_EQ(tex.height(), h);
        EXPECT_EQ(tex.mipLevels(), std::floor(std::log2(std::max(w, h))) + 1);
    }
}

} // namespace vv::test

