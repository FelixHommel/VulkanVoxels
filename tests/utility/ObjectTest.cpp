#include "utility/object/Object.hpp"
#include "utility/Model.hpp"
#include "utility/object/IdPool.hpp"
#include "utility/object/ObjectBuilder.hpp"
#include "utility/object/components/ColorComponent.hpp"
#include "utility/object/components/ModelComponent.hpp"
#include "utility/object/components/PointLightComponent.hpp"
#include "utility/object/components/TransformComponent.hpp"

#include "gtest/gtest.h"

#include <memory>
#include <utility>

namespace vv::test
{

class ModelMock : public Model
{};

class ObjectTest : public ::testing::Test
{
protected:
    void SetUp() override { Object::resetIdPool(); }
};

TEST_F(ObjectTest, ContinuousIds)
{
    const Object o1{};
    const Object o2{};
    EXPECT_LT(o1.getId(), o2.getId());

    const Object o3{ ObjectBuilder().build() };
    const Object o4{ ObjectBuilder().build() };
    EXPECT_LT(o3.getId(), o4.getId());
}

TEST_F(ObjectTest, ObjectMapOperations)
{
    Object::ObjectMap map{};

    Object o1{};
    const ObjectId_t id1{ o1.getId() };
    Object o2{};
    const ObjectId_t id2{ o2.getId() };

    map.emplace(o1.getId(), std::move(o1));
    map.emplace(o2.getId(), std::move(o2));

    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.contains(id1));
    EXPECT_EQ(id1, map.at(id1).getId());
    EXPECT_TRUE(map.contains(id2));
    EXPECT_EQ(id2, map.at(id2).getId());
    EXPECT_FALSE(map.contains(id2 + 1));

    map.erase(map.find(id2));

    EXPECT_EQ(map.size(), 1);
    EXPECT_TRUE(map.contains(id1));
    EXPECT_EQ(id1, map.at(id1).getId());
    EXPECT_FALSE(map.contains(id2));
}

TEST_F(ObjectTest, ObjectAddComponentFromExistingComponent)
{
    constexpr auto color{ glm::vec3(1.f) };
    auto component{ std::make_unique<ColorComponent>(color) };
    Object o{};
    o.addComponent<ColorComponent>(std::move(component));

    EXPECT_TRUE(o.hasComponent<ColorComponent>());
    EXPECT_FALSE(o.hasComponent<ModelComponent>());
    EXPECT_FALSE(o.hasComponent<PointLightComponent>());
    EXPECT_FALSE(o.hasComponent<TransformComponent>());

    EXPECT_EQ(o.getComponent<ColorComponent>()->color, color);
}

TEST_F(ObjectTest, ObjectAddComponentWithComponent)
{
    constexpr auto color{ glm::vec3(1.f) };
    Object o{};
    o.addComponent<ColorComponent>(color);

    EXPECT_TRUE(o.hasComponent<ColorComponent>());
    EXPECT_FALSE(o.hasComponent<ModelComponent>());
    EXPECT_FALSE(o.hasComponent<PointLightComponent>());
    EXPECT_FALSE(o.hasComponent<TransformComponent>());

    EXPECT_EQ(o.getComponent<ColorComponent>()->color, color);
}

TEST_F(ObjectTest, ObjectDoesNotAddSecondComponentOfSameType)
{
    constexpr auto color1{ glm::vec3(1.f) };
    constexpr auto color2{ glm::vec3(0.f) };
    Object o{};
    o.addComponent<ColorComponent>(color1);

    EXPECT_EQ(o.getComponent<ColorComponent>()->color, color1);

    o.addComponent<ColorComponent>(color2);
    EXPECT_NE(o.getComponent<ColorComponent>()->color, color2);
}

TEST_F(ObjectTest, BuilderNoComponents)
{
    const Object o{ ObjectBuilder().build() };

    EXPECT_FALSE(o.hasComponent<ColorComponent>());
    EXPECT_FALSE(o.hasComponent<ModelComponent>());
    EXPECT_FALSE(o.hasComponent<PointLightComponent>());
    EXPECT_FALSE(o.hasComponent<TransformComponent>());
}

TEST_F(ObjectTest, BuilderWithColor)
{
    constexpr auto color{ glm::vec3(1.f) };
    const Object o{ ObjectBuilder().withColor(color).build() };

    EXPECT_TRUE(o.hasComponent<ColorComponent>());
    EXPECT_FALSE(o.hasComponent<ModelComponent>());
    EXPECT_FALSE(o.hasComponent<PointLightComponent>());
    EXPECT_FALSE(o.hasComponent<TransformComponent>());

    EXPECT_EQ(o.getComponent<ColorComponent>()->color, color);
}

TEST_F(ObjectTest, BuilderWithModel)
{
    const std::shared_ptr<Model> model{ std::make_shared<ModelMock>() };
    const Object o{ ObjectBuilder().withModel(model).build() };

    EXPECT_FALSE(o.hasComponent<ColorComponent>());
    EXPECT_TRUE(o.hasComponent<ModelComponent>());
    EXPECT_FALSE(o.hasComponent<PointLightComponent>());
    EXPECT_FALSE(o.hasComponent<TransformComponent>());

    EXPECT_EQ(o.getComponent<ModelComponent>()->model, model);
}

TEST_F(ObjectTest, BuilderWithPointLight)
{
    const Object o{ ObjectBuilder().withPointLight().build() };

    EXPECT_FALSE(o.hasComponent<ColorComponent>());
    EXPECT_FALSE(o.hasComponent<ModelComponent>());
    EXPECT_TRUE(o.hasComponent<PointLightComponent>());
    EXPECT_FALSE(o.hasComponent<TransformComponent>());
}

TEST_F(ObjectTest, BuilderWithTransform)
{
    const Object o{ ObjectBuilder().withTransform().build() };

    EXPECT_FALSE(o.hasComponent<ColorComponent>());
    EXPECT_FALSE(o.hasComponent<ModelComponent>());
    EXPECT_FALSE(o.hasComponent<PointLightComponent>());
    EXPECT_TRUE(o.hasComponent<TransformComponent>());
}

TEST_F(ObjectTest, BuilderWithMultipleComponents)
{
    const Object o{ ObjectBuilder().withTransform().withPointLight().build() };

    EXPECT_FALSE(o.hasComponent<ColorComponent>());
    EXPECT_FALSE(o.hasComponent<ModelComponent>());
    EXPECT_TRUE(o.hasComponent<PointLightComponent>());
    EXPECT_TRUE(o.hasComponent<TransformComponent>());
}


TEST_F(ObjectTest, GetNonExistentComponent)
{
    const auto o{ ObjectBuilder().build() };

    // NOTE: non-const member function
    EXPECT_EQ(o.getComponent<ColorComponent>(), nullptr);

    // NOTE: const member function
    const auto* const constResult{ o.getComponent<ColorComponent>() };
    EXPECT_EQ(constResult, nullptr);
}

} // namespace vv::test
