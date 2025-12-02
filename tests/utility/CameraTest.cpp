#include "utility/Camera.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "gtest/gtest.h"

#include <memory>

namespace vv::test
{

class CameraTest : public ::testing::Test
{
public:
	CameraTest() = default;
	~CameraTest() override = default;

	CameraTest(const CameraTest&) = delete;
	CameraTest(CameraTest&&) = delete;
	CameraTest& operator=(const CameraTest&) = delete;
	CameraTest& operator=(CameraTest&&) = delete;

	void SetUp() override { m_camera = std::make_unique<Camera>(); }
	void TearDown() override {}

protected:
	std::unique_ptr<Camera> m_camera;
};

TEST_F(CameraTest, PerspectiveProjection)
{
    static constexpr float FOV{ glm::radians(45.f) };
    static constexpr float ASPECT_RATIO{ 16.f / 9.f };
    static constexpr float NEAR_PLANE{ 0.1f };
    static constexpr float FAR_PLANE{ 100.f };

	m_camera->setPerspectiveProjection(FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

    const glm::mat4 proj{ m_camera->getProjection() };

    EXPECT_EQ(proj[2][3], 1.f);
}

} // namespace vv::test
