#ifndef SRC_ENGINE_OBJECT_HPP
#define SRC_ENGINE_OBJECT_HPP

#include "Model.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include <memory>

namespace vv
{

struct RigidBody2dComponent
{
    glm::vec2 velocity{};
    float mass{ 1.f };
};

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
    [[nodiscard]] glm::mat4 mat4() const noexcept
    {
        const float c3{ glm::cos(rotation.z) };
        const float s3{ glm::sin(rotation.z) };
        const float c2{ glm::cos(rotation.x) };
        const float s2{ glm::sin(rotation.x) };
        const float c1{ glm::cos(rotation.y) };
        const float s1{ glm::sin(rotation.y) };

        return glm::mat4{
            {
                scale.x * (c1 * c3 + s1 * s2 * s3),
                scale.x * (c2 * s3),
                scale.x * (c1 * s2 * s3 - c3 * s1),
                0.f
            },
            {
                scale.y * (c3 * s1 * s2 - c1 * s3),
                scale.y * (c2 * c3),
                scale.y * (c1 * c3 * s2 + s1 * s3),
                0.f
            },
            {
                scale.z * (c2 * s1),
                scale.z * (-s2),
                scale.z * (c1 * c2),
                0.f
            },
            {
                translation.x,
                translation.y,
                translation.z,
                1.f
            }
        };
    }
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

#endif // !SRC_ENGINE_OBJECT_HPP
