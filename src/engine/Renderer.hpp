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

/// \brief The Renderer provides functionality to draw frames to the screen
///
/// It manages synchronization between frame buffers on a user level and manages
/// the beginning of frames and render passes as well as the ending of frames and render passes
///
/// \author Felix Hommel
/// \date 11/19/2025
class Renderer
{
public:
    /// \brief Create a new \ref Renderer
    ///
    /// \param window the \ref Window that is rendered to
    /// \param device the \ref Device that is used
    Renderer(Window& window, Device& device);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    [[nodiscard]] VkRenderPass getRenderPass() const noexcept { return m_swapchain->getRenderPass(); }
    [[nodiscard]] float getAspectRatio() const noexcept { return m_swapchain->extentAspectRatio(); }
    [[nodiscard]] bool isFrameStarted() const noexcept { return m_isFrameStarted; }
    [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const;
    [[nodiscard]] std::size_t getFrameIndex() const;

    /// \brief Prepares the command buffer for the next frame
    ///
    /// Ensures that the next frame is properly synchronized and ready to be rendered
    /// and then provides the command buffer that is used to render the next frame
    ///
    /// \return VkCommandBuffer that is used to render the next frame
    VkCommandBuffer beginFrame();
    /// \brief End the command buffer recording and submit the finished command buffer
    void endFrame();
    /// \brief Start a new render pass
    ///
    /// Start a new Render pass and then configure the viewport and scissor
    ///
    /// \param commandBuffer currently used VkCommandBuffer
    void beginRenderPass(VkCommandBuffer commandBuffer) const;
    /// \brief End the current render pass
    ///
    /// \param commandBuffer currently used VkCommandBuffer
    void endRenderPass(VkCommandBuffer commandBuffer) const;

private:
    static constexpr VkClearColorValue CLEAR_COLOR{ {0.1f, 0.1f, 0.1f, 1.f} };

    Window& window;
    Device& device;
    std::unique_ptr<Swapchain> m_swapchain;
    std::vector<VkCommandBuffer> m_commandBuffers;

    std::uint32_t m_currentImageIndex{};
    std::size_t m_currentFrameIndex{ 0 };
    bool m_isFrameStarted{ false };

    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapchain();
};

} // !vv

#endif // !SRC_ENGINE_RENDERER_HPP
