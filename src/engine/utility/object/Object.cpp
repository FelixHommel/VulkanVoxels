#include "Object.hpp"

#include "utility/object/IdPool.hpp"

#include <utility>

namespace vv
{

Object::Object()
    : m_id{ s_idPool.acquire() }
{
}

Object::Object(Object&& other) noexcept
    : m_id(other.m_id)
    , m_components(std::move(other.m_components))
{
    other.m_id = INVALID_OBJECT_ID;
    other.m_components.clear();
}

Object& Object::operator=(Object&& other) noexcept
{
    m_id = other.m_id;
    other.m_id = INVALID_OBJECT_ID;

    m_components = std::move(other.m_components);
    other.m_components.clear();

    return *this;
}

} // namespace vv
