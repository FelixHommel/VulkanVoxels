#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_FRAME_INFO_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_FRAME_INFO_HPP

#include "utility/Camera.hpp"
#include "utility/Object.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include <vulkan/vulkan_core.h>

#include <array>
#include <cstddef>
#include <memory>

namespace vv
{

/// \brief Defines a point light by position and color of its light
///
/// \author Felix Hommel
/// \date 11/28/2025
struct PointLight
{
	glm::vec4 position{};
	glm::vec4 color{};
};

static constexpr std::size_t MAX_LIGHTS{ 10 };
static constexpr float AMBIENT_LIGHT_INTENSITY{ 0.02f };

/// \brief Definition of the Global UBO that contains generally relevant data
///
/// \author Felix Hommel
/// \date 11/28.2025
struct GlobalUBO
{
	glm::mat4 projection{ 1.f };
	glm::mat4 view{ 1.f };
	glm::mat4 inverseView{ 1.f };
	glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, AMBIENT_LIGHT_INTENSITY };
	std::array<PointLight, MAX_LIGHTS> pointLights;
	int numLights;
};

/// \brief FrameInfo is a collection of relevant data that regards the entire Frame
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
	std::shared_ptr<Camera> camera;
	VkDescriptorSet globalDescriptorSet;
	std::shared_ptr<Object::ObjectMap> objects;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_FRAME_INFO_HPP
