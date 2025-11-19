#ifndef SRC_ENGINE_CAMERA_HPP
#define SRC_ENGINE_CAMERA_HPP

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
    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
    /// \brief Use a Perspective projection
    ///
    /// \param fovy FOV on the y plane
    /// \param aspectRatio aspect ratio of the framebuffer
    /// \param near near clipping plane
    /// \param far far clipping plane
    void setPerspectiveProjection(float fovy, float aspectRatio, float near, float far);

    const glm::mat4& getProjection() const noexcept { return m_projectionMatrix; }

private:
    glm::mat4 m_projectionMatrix{ 1.f };
};

} // !vv

#endif // !SRC_ENGINE_CAMERA_HPP
