#include "glm/ext/matrix_transform.hpp"
#include "utility/Object.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "gtest/gtest.h"

#include <memory>

namespace vv::test
{

class TransformComponentTest : public ::testing::Test
{
public:
	TransformComponentTest() = default;
	~TransformComponentTest() override = default;

	TransformComponentTest(const TransformComponentTest&) = delete;
	TransformComponentTest(TransformComponentTest&&) = delete;
	TransformComponentTest& operator=(const TransformComponentTest&) = delete;
	TransformComponentTest& operator=(TransformComponentTest&&) = delete;

	void SetUp() override { m_transform = std::make_unique<TransformComponent>(); }
	void TearDown() override {}

protected:
	std::unique_ptr<TransformComponent> m_transform;
};

TEST_F(TransformComponentTest, CalculateModelMatrix)
{
    static constexpr glm::vec3 TRANSLATION{ glm::vec3(1.f, 2.f, 3.f) };
    static constexpr glm::vec3 SCALE{ glm::vec3(2.f) };
    static constexpr glm::vec3 ROTATION{ glm::vec3(0.f) };

    m_transform->translation = TRANSLATION;
    m_transform->scale = SCALE;
    m_transform->rotation = ROTATION;

    const glm::mat4 mat{ m_transform->mat4() };

    // NOTE: Check translation
    EXPECT_EQ(mat[3][0], TRANSLATION[0]);
    EXPECT_EQ(mat[3][1], TRANSLATION[1]);
    EXPECT_EQ(mat[3][2], TRANSLATION[2]);

    // NOTE: Check scale
    EXPECT_EQ(mat[0][0], SCALE[0]);
    EXPECT_EQ(mat[1][1], SCALE[1]);
    EXPECT_EQ(mat[2][2], SCALE[2]);
}

TEST_F(TransformComponentTest, CalculateNormals)
{
    static constexpr glm::vec3 SCALE{ glm::vec3(2.f, 3.f, 4.f) };

    m_transform->scale = SCALE;

    const glm::mat4 normalMat{ m_transform->normalMatrix() };

    static constexpr float ERR{ 0.01f };
    EXPECT_NEAR(normalMat[0][0], 0.5f, ERR);
    EXPECT_NEAR(normalMat[1][1], 0.33f, ERR);
    EXPECT_NEAR(normalMat[2][2], 0.25f, ERR);
}

} // namespace vv::test
