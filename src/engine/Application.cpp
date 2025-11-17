#include "Application.hpp"

#include "BasicRenderSystem.hpp"
#include "MagnetPhysicsSystem.hpp"
#include "Object.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "GLFW/glfw3.h"
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

namespace vv
{

Application::Application()
{
    loadObjects();
}

void Application::run()
{
    std::shared_ptr<Model> squareModel{ createSquareModel(m_device, {.5f, 0.f}) };
    std::shared_ptr<Model> circleModel{ createCircleModel(m_device, 64) };

    std::vector<Object> physicsObjects{};
    auto red{ Object::createObject() };
    red.transform2d.scale = glm::vec2{ 0.05f };
    red.transform2d.translation = { 0.5f, 0.5f };
    red.color = { 1.f, 0.f, 0.f };
    red.rigidBody2d.velocity = { -0.5f, 0.f };
    red.model = circleModel;
    physicsObjects.push_back(std::move(red));
    auto blue{ Object::createObject() };
    blue.transform2d.scale = glm::vec2{ 0.05f };
    blue.transform2d.translation = { -0.45f, -0.25f };
    blue.color = { 0.f, 0.f, 1.f };
    blue.rigidBody2d.velocity = { -0.5f, 0.f };
    blue.model = circleModel;
    physicsObjects.push_back(std::move(blue));

    std::vector<Object> vectorField{};
    const int gridCount{ 40 };
    for(int i{ 0 }; i < gridCount; ++i)
    {
        for(int j{ 0 }; j < gridCount; ++j)
        {
            auto vf{ Object::createObject() };
            vf.transform2d.scale = glm::vec2(0.005f);
            vf.transform2d.translation = {
                -1.f + (static_cast<float>(i) + 0.5f) * 2.f / gridCount,
                -1.f + (static_cast<float>(j) + 0.5f) * 2.f / gridCount
            };
            vf.color = glm::vec3(1.f);
            vf.model = squareModel;

            vectorField.push_back(std::move(vf));
        }
    }

    MagnetPhysicsSystem magnetSystem{ 0.81f };
    Vec2FieldSystem vecFieldSystem{};
    BasicRenderSystem basicRenderSystem{ m_device, m_renderer.getRenderPass() };

    while(!m_window.shouldClose())
    {
        glfwPollEvents();

        if(auto commandBuffer{ m_renderer.beginFrame() })
        {
            magnetSystem.update(physicsObjects, 1.f / 60, 5);
            vecFieldSystem.update(magnetSystem, physicsObjects, vectorField);

            m_renderer.beginRenderPass(commandBuffer);

            basicRenderSystem.renderObjects(commandBuffer, physicsObjects);
            basicRenderSystem.renderObjects(commandBuffer, vectorField);

            m_renderer.endRenderPass(commandBuffer);
            m_renderer.endFrame();
        }
    }

    vkDeviceWaitIdle(m_device.device());
}

void Application::loadObjects()
{
}

std::unique_ptr<Model> Application::createSquareModel(Device& device, glm::vec2 offset)
{
    std::vector<Model::Vertex> vertices{
      {{-0.5f, -0.5f}},
      {{0.5f, 0.5f}},
      {{-0.5f, 0.5f}},
      {{-0.5f, -0.5f}},
      {{0.5f, -0.5f}},
      {{0.5f, 0.5f}}
    };

    for(auto& v : vertices)
        v.position += offset;

    return std::make_unique<Model>(device, vertices);
}

std::unique_ptr<Model> Application::createCircleModel(Device& device, unsigned int numSides)
{
    std::vector<Model::Vertex> uniqueVertices{};

    for(unsigned int i{ 0 }; i < numSides; ++i)
    {
        float angle{ i * glm::two_pi<float>() / numSides };
        uniqueVertices.push_back({{glm::cos(angle), glm::sin(angle)}});
    }
    uniqueVertices.push_back({}); // NOTE: For center vertex (0, 0)

    std::vector<Model::Vertex> vertices{};
    for(std::size_t i{ 0 }; i < numSides; ++i)
    {
        vertices.push_back(uniqueVertices[i]);
        vertices.push_back(uniqueVertices[(i + 1) % numSides]);
        vertices.push_back(uniqueVertices[numSides]);
    }

    return std::make_unique<Model>(device, vertices);
}

} // !vv
