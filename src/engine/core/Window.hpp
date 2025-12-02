#ifndef VULKAN_VOXELS_SRC_ENGINE_CORE_WINDOW_HPP
#define VULKAN_VOXELS_SRC_ENGINE_CORE_WINDOW_HPP

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <string>

namespace vv
{

/// \brief The Window class is a wrapper around the GLFW windowing utility
///
/// \author Felix Hommel
/// \date 11/9/2025
class Window
{
public:
	Window(std::uint32_t width, std::uint32_t height, std::string title);
	~Window();

	Window(const Window&) = delete;
	Window(Window&&) = delete;
	Window& operator=(const Window&) = delete;
	Window& operator=(Window&&) = delete;

	[[nodiscard]] GLFWwindow* getHandle() const noexcept { return m_window; }
	[[nodiscard]] bool shouldClose() const
	{
		return glfwWindowShouldClose(m_window) != 0;
	}
	[[nodiscard]] VkExtent2D getExtent() const noexcept
	{
		return { .width = m_width, .height = m_height };
	}
	[[nodiscard]] bool wasWindowResized() const noexcept
	{
		return m_wasResized;
	}
	void resetWindowResizeFlag() { m_wasResized = false; }

	/// \brief Create the surface that is needed for vulkan rendering
	///
	/// \param instance the VkInstance that is used
	/// \param surface where to store the surface
	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const;

private:
	std::uint32_t m_width;
	std::uint32_t m_height;
	bool m_wasResized{ false };
	std::string m_title;
	GLFWwindow* m_window;

	static void framebufferResizeCallback(
		GLFWwindow* window,
		int width,
		int height
	);
};

} // namespace vv

#endif //! VULKAN_VOXELS_SRC_ENGINE_CORE_WINDOW_HPP
