#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_HPP

#include "Model.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include <memory>

namespace vv
{

/// \brief Component to represent position and transformation in 3D Space
///
/// \author Felix Hommel
/// \date 11/19/2025
struct TransformComponent
{
    glm::vec3 translation{};
    glm::vec3 scale{ 1.f, 1.f, 1.f };
    glm::vec3 rotation{};

    /// \brief Calculate the final matrix that represents the total transformation
    ///
    /// Matrix transformation is calculated with translation * rot.y * rot.x * rot.z * scale
    ///
    /// \return glm::mat4 the calculated model matrix
    [[nodiscard]] glm::mat4 mat4() const noexcept;

    /// \brief Calculate the normal matrix so that the shader doesn't have to do it on a per vertex basis
    ///
    /// \return glm::mat3 the calculated normal matrix
    [[nodiscard]] glm::mat3 normalMatrix() const noexcept;
};

/// \brief An Object is used to represent arbitrary vertex data as objects and associate positions and other properties with them
///
/// \author Felix Hommel
/// \date 11/19/2025
class Object
{
public:
    using id_t = std::uint16_t;

    Object() : m_id{ s_nextId++ } {}

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&&) = default;
    Object& operator=(Object&&) = default;

    [[nodiscard]] id_t getId() const noexcept { return m_id; }

    std::shared_ptr<Model> model{};
    glm::vec3 color{};
    TransformComponent transform{};

private:
    id_t m_id;
    inline static id_t s_nextId{ 0 }; // NOTE: This means that currently there can be a max of 65535 Objects
};

} // !vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_HPP
