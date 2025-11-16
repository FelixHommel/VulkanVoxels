#ifndef SRC_ENGINE_RENDERER_HPP
#define SRC_ENGINE_RENDERER_HPP

#include "Device.hpp"
#include "Swapchain.hpp"
#include "Window.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>

namespace vv
{

class Renderer
{
public:
    Renderer(Window& window, Device& device);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    [[nodiscard]] VkRenderPass getRenderPass() const noexcept { return m_swapchain->getRenderPass(); }
    [[nodiscard]] bool isFrameStarted() const noexcept { return m_isFrameStarted; }
    [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const;
    [[nodiscard]] std::size_t getFrameIndex() const;

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginRenderPass(VkCommandBuffer commandBuffer);
    void endRenderPass(VkCommandBuffer commandBuffer);

private:
    static constexpr VkClearColorValue CLEAR_COLOR{ {0.1f, 0.1f, 0.1f, 1.f} };

    Window& window;
    Device& device;
    std::unique_ptr<Swapchain> m_swapchain;
    std::vector<VkCommandBuffer> m_commandBufers;

    std::uint32_t m_currentImageIndex;
    std::size_t m_currentFrameIndex{ 0 };
    bool m_isFrameStarted{ false };

    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapchain();
};

} // !vv

#endif // !SRC_ENGINE_RENDERER_HPP
