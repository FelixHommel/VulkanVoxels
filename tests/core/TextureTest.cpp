#include "fixtures/TestVulkanContext.hpp"
#include "core/Texture2D.hpp"

#include "gtest/gtest.h"
#include <vulkan/vulkan_core.h>

#include <memory>

namespace vv::test
{

class TextureTest : public ::testing::Test
{
public:
    static constexpr auto TEST_TEXTURE_PATH{ PROJECT_ROOT "resources/textures/brickwall/Poliigon_BrickWallReclaimed_8320_BaseColor.jpg" };
    void SetUp() override { ctx = std::make_unique<TestVulkanContext>(); }

    std::unique_ptr<TestVulkanContext> ctx;
};

TEST_F(TextureTest, createTexture)
{
    Texture2D texture(ctx->device(), TEST_TEXTURE_PATH);
    
    ASSERT_NE(texture.image(), VK_NULL_HANDLE);
}

} // namespace vv::test

