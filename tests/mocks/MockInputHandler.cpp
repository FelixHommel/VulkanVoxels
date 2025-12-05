#include "MockInputHandler.hpp"

namespace vv::test
{

bool MockInputHandler::isKeyPressed(int key) const
{
    const auto it{ m_keyStates.find(key) };
    return it != m_keyStates.cend() && it->second;
}

} // namespace vv::test
