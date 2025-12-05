#include "utility/Model.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "gtest/gtest.h"

namespace vv::test
{

TEST(VertexTest, VertexOperatorEquals)
{
    constexpr auto POSITION{ glm::vec3{ 1.f, 2.f, 2.f } };
    constexpr auto COLOR{ glm::vec3{ 1.f, 1.f, 1.f } };
    constexpr auto NORMAL{ glm::vec3{ 0.f, 1.f, 0.f } };
    constexpr auto UV{ glm::vec2{ 0.5f, 0.5f } };
    constexpr auto POSITION_OFFSET_X{ 1.1f };

    Model::Vertex v1{
        .position = POSITION,
        .color = COLOR,
        .normal = NORMAL,
        .uv = UV
    };

    Model::Vertex v2 = v1;

    EXPECT_EQ(v1, v2);

    v2.position.x = POSITION_OFFSET_X;

    EXPECT_NE(v1, v2);
}

} // namespace VV::test
