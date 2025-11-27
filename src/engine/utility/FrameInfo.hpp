#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_FRAME_INFO_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_FRAME_INFO_HPP

#include "utility/Camera.hpp"
#include "utility/Object.hpp"

#include <vulkan/vulkan_core.h>

#include <cstddef>

namespace vv
{

/// \brief FrameInfo is a collection of relevant data that regards the entire Frme
///
/// Currently is collecting:
/// - frameIndex
/// - delta time
/// - command buffer in use
/// - camera
/// - descriptor set
/// - frame objects
///
/// \author Felix Hommel
/// \date 11/20/2025
struct FrameInfo
{
    std::size_t frameIndex;
    float dt;
    VkCommandBuffer commandBuffer;
    Camera& camera; // NOLINT
    VkDescriptorSet gloablDescriptorSet;
    Object::ObjectMap& objects; // NOLINT
};

} // !vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_FRAME_INFO_HPP
