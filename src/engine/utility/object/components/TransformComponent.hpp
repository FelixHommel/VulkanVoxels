#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_TRANSFORM_COMPONENT_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_TRANSFORM_COMPONENT_HPP

#include "utility/object/components/Component.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

namespace vv
{

/// \brief Component to represent position and transformation in 3D Space
///
/// \author Felix Hommel
/// \date 11/19/2025
struct TransformComponent final : public Component
{
    constexpr explicit TransformComponent(
        const glm::vec3& translation = {}, const glm::vec3& scale = glm::vec3(1.f), const glm::vec3& rotation = {}
    )
        : translation(translation), scale(scale), rotation(rotation)
    {}

    /// \brief Calculate the final matrix that represents the total transformation
    ///
    /// Matrix transformation is calculated with translation * rot.y * rot.x * rot.z * scale
    ///
    /// \return glm::mat4 the calculated model matrix
    [[nodiscard]] glm::mat4 mat4() const noexcept
    {
        const float c3{ glm::cos(rotation.z) };
        const float s3{ glm::sin(rotation.z) };
        const float c2{ glm::cos(rotation.x) };
        const float s2{ glm::sin(rotation.x) };
        const float c1{ glm::cos(rotation.y) };
        const float s1{ glm::sin(rotation.y) };

        return glm::mat4{
            { scale.x * (c1 * c3 + s1 * s2 * s3), scale.x * (c2 * s3), scale.x * (c1 * s2 * s3 - c3 * s1), 0.f },
            { scale.y * (c3 * s1 * s2 - c1 * s3), scale.y * (c2 * c3), scale.y * (c1 * c3 * s2 + s1 * s3), 0.f },
            { scale.z * (c2 * s1),                scale.z * (-s2),     scale.z * (c1 * c2),                0.f },
            { translation.x,                      translation.y,       translation.z,                      1.f }
        };
    }

    /// \brief Calculate the normal matrix so that the shader doesn't have to do it on a per vertex basis
    ///
    /// \return glm::mat3 the calculated normal matrix
    [[nodiscard]] glm::mat3 normalMatrix() const noexcept
    {
        const float c3{ glm::cos(rotation.z) };
        const float s3{ glm::sin(rotation.z) };
        const float c2{ glm::cos(rotation.x) };
        const float s2{ glm::sin(rotation.x) };
        const float c1{ glm::cos(rotation.y) };
        const float s1{ glm::sin(rotation.y) };
        const glm::vec3 invScale{ 1.f / scale };

        return glm::mat3{
            {
             invScale.x * (c1 * c3 + s1 * s2 * s3),
             invScale.x * (c2 * s3),
             invScale.x * (c1 * s2 * s3 - c3 * s1),
             },
            {
             invScale.y * (c3 * s1 * s2 - c1 * s3),
             invScale.y * (c2 * c3),
             invScale.y * (c1 * c3 * s2 + s1 * s3),
             },
            {
             invScale.z * (c2 * s1),
             invScale.z * (-s2),
             invScale.z * (c1 * c2),
             }
        };
    }

    glm::vec3 translation;
    glm::vec3 scale;
    glm::vec3 rotation;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_TRANSFORM_COMPONENT_HPP
