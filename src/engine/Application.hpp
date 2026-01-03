#ifndef VULKAN_VOXELS_SRC_ENGINE_APPLICATION_HPP
#define VULKAN_VOXELS_SRC_ENGINE_APPLICATION_HPP

#include "core/DescriptorPool.hpp"
#include "core/Device.hpp"
#include "core/Renderer.hpp"
#include "core/Window.hpp"
#include "renderSystems/BasicRenderSystem.hpp"
#include "renderSystems/PBRRenderSystem.hpp"
#include "renderSystems/PointLightRenderSystem.hpp"
#include "utility/Scene.hpp"

#include <cstdint>
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
    static constexpr auto CUBE_PATH{ PROJECT_ROOT "resources/models/cube.obj" };
    static constexpr auto SPHERE_PATH{ PROJECT_ROOT "resources/models/sphere.obj" };
    static constexpr auto DRAGON_VASE_PATH{ PROJECT_ROOT "resources/models/dragon.obj" };
    static constexpr auto QUAD_PATH{ PROJECT_ROOT "resources/models/quad.obj" };
    static constexpr auto POINT_LIGHT_INTENSITY{ 0.2f };
    static constexpr auto CAMERA_START_OFFSET_Z{ -2.5f };
    static constexpr auto MATERIAL_ALBEDO_PATH{ PROJECT_ROOT "resources/textures/worn-shiny-metal-bl/worn-shiny-metal_albedo.png" };
    static constexpr auto MATERIAL_NORMAL_PATH{ PROJECT_ROOT "resources/textures/worn-shiny-metal-bl/worn-shiny-metal_Normal-ogl.png" };
    static constexpr auto MATERIAL_METALLIC_ROUGHNESS_PATH{ PROJECT_ROOT
                                                            "resources/textures/worn-shiny-metal-bl/worn-shiny-metal_metallicRoughness.png" };
    static constexpr auto MATERIAL_OCCLUSION_PATH{ PROJECT_ROOT "resources/textures/worn-shiny-metal-bl/worn-shiny-metal_ao.png" };

    std::shared_ptr<Window> m_window;
    std::shared_ptr<Device> m_device;
    std::unique_ptr<DescriptorPool> m_globalPool;
    std::unique_ptr<Renderer> m_renderer;
    std::vector<std::unique_ptr<Buffer>> m_uboBuffers;
    std::shared_ptr<DescriptorSetLayout> m_globalSetLayout;
    std::vector<VkDescriptorSet> m_globalDescriptorSets;
    std::unique_ptr<BasicRenderSystem> m_basicRenderSystem;
    std::unique_ptr<PointLightRenderSystem> m_pointLightRenderSystem;
    std::unique_ptr<PBRRenderSystem> m_pbrRenderSystem;
    std::unique_ptr<Scene> m_scene;

    void initScene();
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_APPLICATION_HPP
