#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_IINPUT_HANDLER_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_IINPUT_HANDLER_HPP

namespace vv
{

class IInputHandler
{
public:
    IInputHandler() = default;
    virtual ~IInputHandler() = default;

    IInputHandler(const IInputHandler&) = default;
    IInputHandler(IInputHandler&&) = default;
    IInputHandler& operator=(const IInputHandler&) = default;
    IInputHandler& operator=(IInputHandler&&) = default;

    [[nodiscard]] virtual bool isKeyPressed(int key) const = 0;
    [[nodiscard]] virtual bool shouldClose() const = 0;
    virtual void setShouldClose(bool close) = 0;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_IINPUT_HANDLER_HPP
