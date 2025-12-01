#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_CAMERA_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_CAMERA_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

namespace vv
{

/// \brief The Camera class is used to represent the point of view in the current scene
///
/// \author Felix Hommel
/// \date 11/19/2025
class Camera
{
public:
	/// \brief Use an Orthographic projection
	///
	/// \param left left plane
	/// \param right right plane
	/// \param top top plane
	/// \param bottom bottom plane
	/// \param near near plane
	/// \param far far plane
	void setOrthographicProjection(
		float left,
		float right,
		float top,
		float bottom,
		float near,
		float far
	);
	/// \brief Use a Perspective projection
	///
	/// \param fovy FOV on the y plane
	/// \param aspectRatio aspect ratio of the framebuffer
	/// \param near near clipping plane
	/// \param far far clipping plane
	void setPerspectiveProjection(
		float fovy,
		float aspectRatio,
		float near,
		float far
	);
	/// \brief Camera is looking in a specific direction
	///
	/// \param position position of the camera
	/// \param direction direction in which the camera is pointing
	/// \param up which direction is up
	void setViewDirection(
		const glm::vec3& position,
		const glm::vec3& direction,
		const glm::vec3& up = glm::vec3{ 0.f, -1.f, 0.f }
	);
	/// \brief Camera is locked to a spcific point in space
	///
	/// \param position position of the camera
	/// \param target which position to target
	/// \param up which direction is up
	void setViewTarget(
		const glm::vec3& position,
		const glm::vec3& target,
		const glm::vec3& up = glm::vec3{ 0.f, -1.f, 0.f }
	);
	/// \brief Use euler angles to specify the orientation of the camera
	///
	/// \param position position of the camera
	/// \param rotation rotation of the camera
	void setViewXYZ(const glm::vec3& position, const glm::vec3& rotation);

	[[nodiscard]] const glm::mat4& getProjection() const noexcept
	{
		return m_projectionMatrix;
	}
	[[nodiscard]] const glm::mat4& getView() const noexcept
	{
		return m_viewMatrix;
	}
	[[nodiscard]] const glm::mat4& getInverseView() const noexcept
	{
		return m_inverseViewMatrix;
	}

private:
	glm::mat4 m_projectionMatrix{ 1.f };
	glm::mat4 m_viewMatrix{ 1.f };
	glm::mat4 m_inverseViewMatrix{ 1.f };
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_CAMERA_HPP
