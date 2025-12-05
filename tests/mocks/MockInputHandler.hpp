#ifndef VULKAN_VOXELS_TESTS_MOCKS_MOCK_INPUT_HANDLER_HPP
#define VULKAN_VOXELS_TESTS_MOCKS_MOCK_INPUT_HANDLER_HPP

#include "utility/IInputHandler.hpp"
#include <unordered_map>

namespace vv::test
{

class MockInputHandler : public IInputHandler
{
public:
    MockInputHandler() = default;
    ~MockInputHandler() override = default;

    MockInputHandler(const MockInputHandler&) = default;
    MockInputHandler(MockInputHandler&&) = default;
    MockInputHandler& operator=(const MockInputHandler&) = default;
    MockInputHandler& operator=(MockInputHandler&&) = default;

    [[nodiscard]] bool isKeyPressed(int key) const override;
    [[nodiscard]] bool shouldClose() const override { return m_shouldClose; }
    void setShouldClose(bool close) override { m_shouldClose = close; }

    void setKeyPressed(int key, bool pressed) { m_keyStates[key] = pressed; }
    void releaseAllKeys() { m_keyStates.clear(); }

private:
    std::unordered_map<int, bool> m_keyStates;
    bool m_shouldClose{ false };
};

} // namespace vv::test

#endif // !VULKAN_VOXELS_TESTS_MOCKS_MOCK_INPUT_HANDLER_HPP
