#ifndef VULKAN_VOXELS_SRC_ENGINE_APPLICATION_HPP
#define VULKAN_VOXELS_SRC_ENGINE_APPLICATION_HPP

#include "core/Device.hpp"
#include "core/Renderer.hpp"
#include "core/Window.hpp"
#include "utility/Object.hpp"

#include <memory>

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
    ~Application() = default;

    Application(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;

    /// \brief start the Application
    void run();

    static constexpr std::uint32_t WINDOW_WIDTH{ 800 };
    static constexpr std::uint32_t WINDOW_HEIGHT{ 600 };
    static constexpr auto WINDOW_TITLE{ "VulkanVoxels" };

private:
    static constexpr auto SMOOTH_VASE_PATH{ PROJECT_ROOT "resources/models/smooth_vase.obj" };
    static constexpr auto FLAT_VASE_PATH{ PROJECT_ROOT "resources/models/flat_vase.obj" };

    Window m_window{ WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE };
    Device m_device{ m_window };
    Renderer m_renderer{ m_window, m_device };

    std::vector<Object> m_objects;

    void loadObjects();
};

} // !vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_APPLICATION_HPP
