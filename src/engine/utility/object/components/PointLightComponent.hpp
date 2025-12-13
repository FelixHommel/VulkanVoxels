#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_POINT_LIGHT_COMPONENT_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_POINT_LIGHT_COMPONENT_HPP

#include "utility/object/components/Component.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

namespace vv
{

/// \brief Component used by point lights to represent their individual properties
///
/// \author Felix Hommel
/// \date 11/28/2025
struct PointLightComponent final : public Component
{
    static constexpr float DEFAULT_INTENSITY{ 10.f };
    static constexpr auto DEFAULT_COLOR{ glm::vec3(1.f) };
    static constexpr float DEFAULT_RADIUS{ 0.1f };

    PointLightComponent(float intensity, const glm::vec3& color, float radius)
        : intensity{ intensity }, color{ color }, radius{ radius }
    {}

    float intensity;
    glm::vec3 color;
    float radius;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_POINT_LIGHT_COMPONENT_HPP
