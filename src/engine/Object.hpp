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

struct TransformComponent
{
    glm::vec3 translation{};
    glm::vec3 scale{ 1.f, 1.f, 1.f };
    glm::vec3 rotation{};

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

class Object
{
public:
    using id_t = std::uint16_t;

    static Object createObject()
    {
        static id_t currentId{ 0 };
        return Object{ currentId++ };
    }

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&&) = default;
    Object& operator=(Object&&) = default;

    id_t getId() { return m_id; }

    std::shared_ptr<Model> model{};
    glm::vec3 color{};
    TransformComponent transform{};

private:
    Object(id_t objId) : m_id{ objId } {}

    id_t m_id;
};

} // !vv

#endif // !SRC_ENGINE_OBJECT_HPP
