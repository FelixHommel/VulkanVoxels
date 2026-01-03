#include "Application.hpp"

#include "core/Buffer.hpp"
#include "core/DescriptorPool.hpp"
#include "core/DescriptorSetLayout.hpp"
#include "core/DescriptorWriter.hpp"
#include "core/Device.hpp"
#include "core/Renderer.hpp"
#include "core/Swapchain.hpp"
#include "core/Texture2D.hpp"
#include "core/Window.hpp"
#include "renderSystems/BasicRenderSystem.hpp"
#include "renderSystems/PBRRenderSystem.hpp"
#include "renderSystems/PointLightRenderSystem.hpp"
#include "utility/Camera.hpp"
#include "utility/FrameInfo.hpp"
#include "utility/KeyboardMovementController.hpp"
#include "utility/object/Object.hpp"
#include "utility/object/ObjectBuilder.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "GLFW/glfw3.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include <vulkan/vulkan_core.h>

#include <chrono>
#include <cstddef>
#include <memory>
#include <vector>

namespace vv
{

Application::Application()
    : m_window{ std::make_shared<Window>(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE) }
    , m_device{ std::make_shared<Device>(m_window) }
    , m_globalPool{ DescriptorPool::Builder(m_device)
                        .setMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
                        .build() }
    , m_renderer{ std::make_unique<Renderer>(m_window, m_device) }
    , m_uboBuffers(Swapchain::MAX_FRAMES_IN_FLIGHT)
    , m_globalSetLayout{ DescriptorSetLayout::Builder(m_device)
                             .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                             .buildShared() }
    , m_globalDescriptorSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
{
    for(std::size_t i{ 0 }; i < m_uboBuffers.size(); ++i)
        m_uboBuffers[i] = std::make_unique<Buffer>(Buffer::createUniformBuffer(m_device, sizeof(GlobalUBO), 1));

    for(std::size_t i{ 0 }; i < m_globalDescriptorSets.size(); ++i)
    {
        auto bufferInfo{ m_uboBuffers[i]->descriptorInfo() };

        DescriptorWriter(m_globalSetLayout.get(), m_globalPool.get())
            .writeBuffer(0, &bufferInfo)
            .build(m_globalDescriptorSets[i]);
    }

    m_basicRenderSystem = std::make_unique<BasicRenderSystem>(
        m_device, m_renderer->getRenderPass(), m_globalSetLayout->getDescriptorLayout()
    );
    m_pointLightRenderSystem = std::make_unique<PointLightRenderSystem>(
        m_device, m_renderer->getRenderPass(), m_globalSetLayout->getDescriptorLayout()
    );
    m_pbrRenderSystem = std::make_unique<PBRRenderSystem>(
        m_device, m_renderer->getRenderPass(), m_globalSetLayout->getDescriptorLayout()
    );
    m_scene = std::make_unique<Scene>(m_device, m_pbrRenderSystem->getMaterialSetLayout());
    initScene();
}

void Application::run()
{
    std::shared_ptr<Camera> camera{ std::make_shared<Camera>() };
    std::unique_ptr<Object> viewer{ std::make_unique<Object>(ObjectBuilder().withTransform().build()) };
    viewer->getComponent<TransformComponent>()->translation.z = CAMERA_START_OFFSET_Z;

    auto currentTime{ std::chrono::high_resolution_clock::now() };

    while(!m_window->shouldClose())
    {
        glfwPollEvents();

        auto newTime{ std::chrono::high_resolution_clock::now() };
        float dt{ std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count() };
        currentTime = newTime;

        KeyboardMovementController::moveInPlaneXZ(m_window->getHandle(), dt, *viewer);
        camera->setViewXYZ(
            viewer->getComponent<TransformComponent>()->translation,
            viewer->getComponent<TransformComponent>()->rotation
        );

        const float aspectRatio{ m_renderer->getAspectRatio() };
        constexpr float fov{ 50.f };
        constexpr float nearPlane{ 0.1f };
        constexpr float farPlane{ 100.f };
        camera->setPerspectiveProjection(glm::radians(fov), aspectRatio, nearPlane, farPlane);

        if(auto* const commandBuffer{ m_renderer->beginFrame() })
        {
            std::size_t frameIndex{ m_renderer->getFrameIndex() };
            FrameInfo frameInfo{ .frameIndex = frameIndex,
                                 .dt = dt,
                                 .commandBuffer = commandBuffer,
                                 .camera = camera,
                                 .globalDescriptorSet = m_globalDescriptorSets[frameIndex],
                                 .objects = m_scene->getObjects(),
                                 .lights = m_scene->getPointLights() };
            GlobalUBO ubo{};
            ubo.projection = camera->getProjection();
            ubo.view = camera->getView();
            ubo.inverseView = camera->getInverseView();

            m_pointLightRenderSystem->update(frameInfo, ubo);

            m_uboBuffers[frameIndex]->writeToBuffer(ubo);

            m_renderer->beginRenderPass(commandBuffer);

            m_pbrRenderSystem->render(frameInfo);
            m_pointLightRenderSystem->render(frameInfo);

            m_renderer->endRenderPass(commandBuffer);
            m_renderer->endFrame();
        }
    }

    vkDeviceWaitIdle(m_device->device());
}

void Application::initScene()
{
    constexpr glm::vec3 OBJ_SACLE{
        glm::vec3{ 0.5f, 0.5f, 0.5f }
    };
    constexpr glm::vec3 OBJ_POS{
        glm::vec3{ 0.f, 0.f, 0.f }
    };
    MaterialConfig matConfigMetal{};
    std::shared_ptr<Texture2D> texture{
        std::make_shared<Texture2D>(Texture2D::loadFromFile(m_device, MATERIAL_ALBEDO_PATH_METAL, TextureConfig::albedo()))
    };
    matConfigMetal.albedoTexture = texture;
    texture
        = std::make_shared<Texture2D>(Texture2D::loadFromFile(m_device, MATERIAL_NORMAL_PATH_METAL, TextureConfig::normal()));
    matConfigMetal.normalTexture = texture;
    texture = std::make_shared<Texture2D>(
        Texture2D::loadFromFile(m_device, MATERIAL_METALLIC_ROUGHNESS_PATH_METAL, TextureConfig::albedo())
    );
    matConfigMetal.metallicRoughnessTexture = texture;
    texture = std::make_shared<Texture2D>(
        Texture2D::loadFromFile(m_device, MATERIAL_OCCLUSION_PATH_METAL, TextureConfig::albedo())
    );
    matConfigMetal.occlusionTexture = texture;

    auto material = m_scene->createMaterial(matConfigMetal);
    std::shared_ptr<Model> model = Model::loadFromFile(m_device, SPHERE_PATH);
    Object smoothVase{
        ObjectBuilder().withModel(model).withTransform(OBJ_POS, OBJ_SACLE).withMaterial(material).build()
    };

    m_scene->addObject(std::move(smoothVase));

    constexpr glm::vec3 floorPos{
        glm::vec3{ 0.f, 0.5f, 0.f }
    };
    constexpr glm::vec3 floorScale{
        glm::vec3{ 3.f, 1.f, 3.f }
    };

    MaterialConfig matConfigBrick{};
    texture =
        std::make_shared<Texture2D>(Texture2D::loadFromFile(m_device, MATERIAL_ALBEDO_PATH_BRICK, TextureConfig::albedo()));
    matConfigMetal.albedoTexture = texture;
    texture
        = std::make_shared<Texture2D>(Texture2D::loadFromFile(m_device, MATERIAL_NORMAL_PATH_BRICK, TextureConfig::normal()));
    matConfigMetal.normalTexture = texture;
    texture = std::make_shared<Texture2D>(
        Texture2D::loadFromFile(m_device, MATERIAL_METALLIC_ROUGHNESS_PATH_BRICK, TextureConfig::albedo())
    );
    matConfigMetal.metallicRoughnessTexture = texture;
    texture = std::make_shared<Texture2D>(
        Texture2D::loadFromFile(m_device, MATERIAL_OCCLUSION_PATH_BRICK, TextureConfig::albedo())
    );
    matConfigMetal.occlusionTexture = texture;
    material = m_scene->createMaterial(matConfigMetal);

    model = Model::loadFromFile(m_device, QUAD_PATH);
    Object floor{ ObjectBuilder().withModel(model).withTransform(floorPos, floorScale).withMaterial(material).build() };
    m_scene->addObject(std::move(floor));

    m_scene->addPointlight(ObjectBuilder().withPointLight(50.f, glm::vec3(1.f, 1.f, 1.f)).withTransform(glm::vec3(2.f, -2.f, -1.f)).build());
    // m_scene->addPointlight(ObjectBuilder().withPointLight(20.f, glm::vec3(0.8f, 0.8f, 1.f)).withTransform(glm::vec3(-2.f, -1.f, 0.f)).build());

    // constexpr auto COLOR_RED{
    //     glm::vec3{ 1.f, 1.f, 1.f }
    // };
    // constexpr auto COLOR_BLUE{
    //     glm::vec3{ 1.f, 1.f, 1.f }
    // };
    // constexpr std::size_t LIGHTS{ 2 };
    // for(std::size_t i{ 0 }; i < LIGHTS; ++i)
    // {
    //     const auto rotateLight{ glm::rotate(
    //         glm::mat4(1.f),
    //         static_cast<float>(i) * glm::two_pi<float>() / static_cast<float>(LIGHTS),
    //         { 0.f, -1.f, 0.f }
    //     ) };
    //     const auto translateLight{ glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f)) };
    //
    //     m_scene->addPointlight(
    //         ObjectBuilder()
    //             .withPointLight(POINT_LIGHT_INTENSITY, i % 2 == 0 ? COLOR_RED : COLOR_BLUE)
    //             .withTransform(translateLight)
    //             .build()
    //     );
    // }
}

} // namespace vv
