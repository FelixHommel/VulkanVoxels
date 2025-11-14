#ifndef SRC_ENGINE_APPLICATION_HPP
#define SRC_ENGINE_APPLICATION_HPP

#include "Swapchain.hpp"
#include "Device.hpp"
#include "Pipeline.hpp"
#include "Window.hpp"

#include <cstdint>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace vv
{

/// \brief The Application coordinates everything to work with each other
/// 
/// \author Felix Hommel
/// \date 11/9/2025
class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;

    /// \brief start the Application
    void run();

    static constexpr std::uint32_t WINDOW_WIDTH{ 800 };
    static constexpr std::uint32_t WINDOW_HEIGHT{ 600 };
    static constexpr auto WINDOW_TITLE{ "VulkanVoxels" };
    static constexpr auto VERTEX_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/simpleVert.spv" };
    static constexpr auto FRAGMENT_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/simpleFrag.spv" };

private:
    Window m_window{ WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE };
    Device m_device{ m_window };
    Swapchain m_swapchain{ m_device, m_window.getExtent() };
    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
    std::vector<VkCommandBuffer> m_commandBufers;

    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();
    void drawFrame();
};

} // !vv

#endif // !SRC_ENGINE_APPLICATION_HPP
