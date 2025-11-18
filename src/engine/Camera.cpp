#include "Camera.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

namespace vv
{

void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far)
{
    // FIXME: When used this doesn't disaply anything.
    m_projectionMatrix = glm::mat4{1.0f};
    m_projectionMatrix[0][0] = 2.f / (right - left);
    m_projectionMatrix[1][1] = 2.f / (bottom - top);
    m_projectionMatrix[2][2] = 1.f / (far - near);
    m_projectionMatrix[3][0] = -(right + left) / (right - left);
    m_projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
    m_projectionMatrix[3][2] = -near / (far - near);
}

void Camera::setPerspectiveProjection(float fovy, float aspectRatio, float near, float far)
{
#if defined(VV_ENABLE_ASSERTS)
    assert(glm::abs(aspectRatio - std::numeric_limits<float>::epsilon()) > 0.0f);
#endif

    const float tanHalfFovy = std::tan(fovy / 2.f);
    m_projectionMatrix = glm::mat4{ 0.f };
    m_projectionMatrix[0][0] = 1.f / (aspectRatio * tanHalfFovy);
    m_projectionMatrix[1][1] = 1.f / tanHalfFovy;
    m_projectionMatrix[2][2] = far / (far - near);
    m_projectionMatrix[2][3] = 1.f;
    m_projectionMatrix[3][2] = -(far * near) / (far - near);
}

} // !vv
