#ifndef VULKAN_VOXELS_SRC_ENGINE_APPLICATION_HPP
#define VULKAN_VOXELS_SRC_ENGINE_APPLICATION_HPP

#include "core/DescriptorPool.hpp"
#include "core/Device.hpp"
#include "core/Renderer.hpp"
#include "core/Window.hpp"
#include "glm/geometric.hpp"
#include "utility/Object.hpp"

#include <cstdint>
#include <memory>

namespace
{
    constexpr glm::vec3 DEFAULT_LIGHT_DIRECTION{ glm::vec3{ 1.f, -3.f, -1.f } };
    constexpr unsigned int UBO_ALIGNMENT{ 16 };
    constexpr float AMBIENT_LIGHT_INTENSITY{ 0.02f };
}

namespace vv
{

struct GloablUBO
{
    glm::mat4 porjection{ 1.f };
    glm::mat4 view{ 1.f };
    glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, ::AMBIENT_LIGHT_INTENSITY };
    glm::vec3 lightPosition{ -1.f };
    alignas(::UBO_ALIGNMENT) glm::vec4 lightColor{ 1.f };
};

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
    static constexpr auto QUAD_PATH{ PROJECT_ROOT "resources/models/quad.obj" };

    Window m_window{ WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE };
    Device m_device{ m_window };
    Renderer m_renderer{ m_window, m_device };

    std::unique_ptr<DescriptorPool> m_globalPool;
    Object::ObjectMap m_objects;

    void loadObjects();
};

} // !vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_APPLICATION_HPP
