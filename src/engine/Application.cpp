#include "Application.hpp"

#include "core/Buffer.hpp"
#include "core/DescriptorPool.hpp"
#include "core/DescriptorSetLayout.hpp"
#include "core/DescriptorWriter.hpp"
#include "core/Device.hpp"
#include "core/Renderer.hpp"
#include "core/Swapchain.hpp"
#include "core/Window.hpp"
#include "renderSystems/BasicRenderSystem.hpp"
#include "renderSystems/PointLightRenderSystem.hpp"
#include "utility/Camera.hpp"
#include "utility/FrameInfo.hpp"
#include "utility/KeyboardMovementController.hpp"
#include "utility/Model.hpp"
#include "utility/Object.hpp"

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
#include <utility>
#include <vector>

namespace vv
{

Application::Application()
{
	m_window =
		std::make_shared<Window>(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
	m_device = std::make_shared<Device>(m_window);
	m_renderer = std::make_unique<Renderer>(m_window, m_device);
	m_globalPool = DescriptorPool::Builder(m_device)
	                   .setMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
	                   .addPoolSize(
						   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						   Swapchain::MAX_FRAMES_IN_FLIGHT
					   )
	                   .build();
	loadObjects();
}

void Application::run()
{
	std::vector<std::unique_ptr<Buffer>> uboBuffers(
		Swapchain::MAX_FRAMES_IN_FLIGHT
	);
	for(std::size_t i{ 0 }; i < uboBuffers.size(); ++i)
	{
		uboBuffers[i] = std::make_unique<Buffer>(
			m_device,
			sizeof(GlobalUBO),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			m_device->properties.limits.minUniformBufferOffsetAlignment
		);
		uboBuffers[i]->map();
	}

	auto globalSetLayout{ DescriptorSetLayout::Builder(m_device)
		                      .addBinding(
								  0,
								  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
								  VK_SHADER_STAGE_ALL_GRAPHICS
							  )
		                      .build() };

	std::vector<VkDescriptorSet> globalDescriptorSets(
		Swapchain::MAX_FRAMES_IN_FLIGHT
	);
	for(std::size_t i{ 0 }; i < globalDescriptorSets.size(); ++i)
	{
		auto bufferInfo{ uboBuffers[i]->descriptorInfo() };

		DescriptorWriter(globalSetLayout.get(), m_globalPool.get())
			.writeBuffer(0, &bufferInfo)
			.build(globalDescriptorSets[i]);
	}

	BasicRenderSystem basicRenderSystem{ m_device,
		                                 m_renderer->getRenderPass(),
		                                 globalSetLayout->getDescriptorLayout(
										 ) };
	PointLightRenderSystem pointLightRenderSystem{
		m_device,
		m_renderer->getRenderPass(),
		globalSetLayout->getDescriptorLayout()
	};
    std::shared_ptr<Camera> camera{ std::make_shared<Camera>() };
	Object viewer{};
	viewer.transform.translation.z = CAMERA_START_OFFSET_Z;

	auto currentTime{ std::chrono::high_resolution_clock::now() };

	while(!m_window->shouldClose())
	{
		glfwPollEvents();

		auto newTime{ std::chrono::high_resolution_clock::now() };
		float dt{ std::chrono::duration<float, std::chrono::seconds::period>(
					  newTime - currentTime
		)
			          .count() };
		currentTime = newTime;

		KeyboardMovementController::moveInPlaneXZ(
			m_window->getHandle(), dt, viewer
		);
		camera->setViewXYZ(
			viewer.transform.translation, viewer.transform.rotation
		);

		const float aspectRatio{ m_renderer->getAspectRatio() };
		constexpr float fov{ 50.f };
		constexpr float nearPlane{ 0.1f };
		constexpr float farPlane{ 100.f };
		camera->setPerspectiveProjection(
			glm::radians(fov), aspectRatio, nearPlane, farPlane
		);

		if(auto* const commandBuffer{ m_renderer->beginFrame() })
		{
			std::size_t frameIndex{ m_renderer->getFrameIndex() };
			FrameInfo frameInfo{ .frameIndex = frameIndex,
				                 .dt = dt,
				                 .commandBuffer = commandBuffer,
				                 .camera = camera,
				                 .globalDescriptorSet =
				                     globalDescriptorSets[frameIndex],
				                 .objects = m_objects };
			GlobalUBO ubo{};
			ubo.projection = camera->getProjection();
			ubo.view = camera->getView();
			ubo.inverseView = camera->getInverseView();

			PointLightRenderSystem::update(frameInfo, ubo);

			uboBuffers[frameIndex]->writeToBuffer(ubo);
			uboBuffers[frameIndex]->flush();

			m_renderer->beginRenderPass(commandBuffer);

			basicRenderSystem.renderObjects(frameInfo);
			pointLightRenderSystem.render(frameInfo);

			m_renderer->endRenderPass(commandBuffer);
			m_renderer->endFrame();
		}
	}

	vkDeviceWaitIdle(m_device->device());
}

/// \brief Load all objects that are being used
void Application::loadObjects()
{
	constexpr glm::vec3 vaseScale{ glm::vec3{ 3.f, 1.5f, 3.f } };
	constexpr glm::vec3 flatVasePos{ glm::vec3{ -0.5f, 0.5f, 0.f } };
	constexpr glm::vec3 smoothVasePos{ glm::vec3{ 0.5f, 0.5f, 0.f } };

	std::shared_ptr<Model> model{
		Model::loadFromFile(m_device, FLAT_VASE_PATH)
	};
	Object flatVase{};
	flatVase.model = model;
	flatVase.transform.translation = flatVasePos;
	flatVase.transform.scale = vaseScale;
	m_objects->emplace(flatVase.getId(), std::move(flatVase));

	model = Model::loadFromFile(m_device, SMOOTH_VASE_PATH);
	Object smoothVase{};
	smoothVase.model = model;
	smoothVase.transform.translation = smoothVasePos;
	smoothVase.transform.scale = vaseScale;
	m_objects->emplace(smoothVase.getId(), std::move(smoothVase));

	constexpr glm::vec3 floorPos{ glm::vec3{ 0.f, 0.5f, 0.f } };
	constexpr glm::vec3 floorScale{ glm::vec3{ 3.f, 1.f, 3.f } };

	model = Model::loadFromFile(m_device, QUAD_PATH);
	Object floor{};
	floor.model = model;
	floor.transform.translation = floorPos;
	floor.transform.scale = floorScale;
	m_objects->emplace(floor.getId(), std::move(floor));

	const std::vector<glm::vec3> lightColors{
		{ 1.f, .1f, .1f }, { .1f, .1f, 1.f }, { .1f, 1.f, .1f },
		{ 1.f, 1.f, .1f }, { .1f, 1.f, 1.f }, { 1.f, 1.f, 1.f }
	};

	for(std::size_t i{ 0 }; i < lightColors.size(); ++i)
	{
		Object pointLight{ Object::makePointLight(POINT_LIGHT_INTENSITY) };
		pointLight.color = lightColors[i];
		auto rotateLight{ glm::rotate(
			glm::mat4(1.f),
			static_cast<float>(i) * glm::two_pi<float>() / static_cast<float>(lightColors.size()),
			{ 0.f, -1.f, 0.f }
		) };

		pointLight.transform.translation =
			glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));

		m_objects->emplace(pointLight.getId(), std::move(pointLight));
	}
}

} // namespace vv
