#ifndef SRC_ENGINE_CAMERA_HPP
#define SRC_ENGINE_CAMERA_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

namespace vv
{

class Camera
{
public:
    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
    void setPerspectiveProjection(float fovy, float aspectRatio, float near, float far);

    const glm::mat4& getProjection() const noexcept { return m_projectionMatrix; }

private:
    glm::mat4 m_projectionMatrix{ 1.f };
};

} // !vv

#endif // !SRC_ENGINE_CAMERA_HPP
