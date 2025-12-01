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
}

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
	static constexpr auto SMOOTH_VASE_PATH{
		PROJECT_ROOT "resources/models/smooth_vase.obj"
	};
	static constexpr auto FLAT_VASE_PATH{ PROJECT_ROOT
		                                  "resources/models/flat_vase.obj" };
	static constexpr auto QUAD_PATH{ PROJECT_ROOT "resources/models/quad.obj" };

	std::shared_ptr<Window> m_window;
	std::shared_ptr<Device> m_device;
	std::unique_ptr<DescriptorPool> m_globalPool;
	std::unique_ptr<Renderer> m_renderer;
	// std::shared_ptr<Window> m_window{ std::make_shared<Window>(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE) };
	// std::shared_ptr<Device> m_device{ std::make_shared<Device>(m_window) };
	// Renderer m_renderer{ m_window, m_device };

	/*
	std::unique_ptr<DescriptorPool> m_globalPool{
	    DescriptorPool::Builder(m_device)
	        .setMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
	        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
	        .build()
	};
	*/
	Object::ObjectMap m_objects;

	void loadObjects();
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_APPLICATION_HPP
