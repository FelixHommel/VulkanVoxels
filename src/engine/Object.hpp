#ifndef SRC_ENGINE_OBJECT_HPP
#define SRC_ENGINE_OBJECT_HPP

#include "Model.hpp"

#include "glm/glm.hpp"

#include <memory>

namespace vv
{

struct Transform2dComponent
{
    glm::vec2 translation{};
    glm::vec2 scale{ 1.f, 1.f };
    float rotation;

    [[nodiscard]] glm::mat2 mat2() const noexcept
    {
        const float s{ glm::sin(rotation) };
        const float c{ glm::cos(rotation) };
        const glm::mat2 rotMatrix{ {c, s}, {-s, c} };

        const glm::mat2 scaleMat{ {scale.x, .0f}, {.0f, scale.y} };

        return rotMatrix * scaleMat;
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
    Transform2dComponent transform2d{};

private:
    Object(id_t objId) : m_id{ objId } {}

    id_t m_id;
};

} // !vv

#endif // !SRC_ENGINE_OBJECT_HPP
