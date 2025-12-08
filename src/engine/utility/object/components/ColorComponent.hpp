#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_COLOR_COMPONENT_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_COLOR_COMPONENT_HPP

#include "utility/object/components/Component.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

namespace vv
{

/// \brief Component that gives an \ref Object a color
///
/// \author Felix Hommel
/// \date 12/7/2025
struct ColorComponent final : public Component
{
    constexpr explicit ColorComponent(const glm::vec3& color) : color(color) {}

    glm::vec3 color;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_COLOR_COMPONENT_HPP