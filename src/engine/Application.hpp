#ifndef SRC_ENGINE_APPLICATION_HPP
#define SRC_ENGINE_APPLICATION_HPP

#include "Object.hpp"
#include "Device.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

#include <memory>
#include <vulkan/vulkan_core.h>

#include <cstdint>

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
    Window m_window{ WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE };
    Device m_device{ m_window };
    Renderer m_renderer{ m_window, m_device };

    std::vector<Object> m_objects;

    void loadObjects();

    std::unique_ptr<Model> createSquareModel(Device& device, glm::vec2 offset);
    std::unique_ptr<Model> createCircleModel(Device& device, unsigned int numSides);
};

} // !vv

#endif // !SRC_ENGINE_APPLICATION_HPP
