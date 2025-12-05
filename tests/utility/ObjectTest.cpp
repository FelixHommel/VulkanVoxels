#include "utility/Object.hpp"

#include "gtest/gtest.h"

#include <utility>

namespace vv::test
{

TEST(ObjectTest, ContinuousIds)
{
	Object o1{};
	Object o2{};

    EXPECT_LT(o1.getId(), o2.getId());
}

TEST(ObjectTest, ContinuousIdWithPointLights)
{
    Object o1{};
    Object o2{ Object::makePointLight() };

    EXPECT_LT(o1.getId(), o2.getId());
}

TEST(ObjectTest, ObjectMapOperations)
{
    Object::ObjectMap map{};

    Object o1{};
    const Object::id_t id1{ o1.getId() };
    Object o2{};
    const Object::id_t id2{ o2.getId() };

    map.emplace(o1.getId(), std::move(o1));
    map.emplace(o2.getId(), std::move(o2));

    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.contains(id1));
    EXPECT_TRUE(map.contains(id2));
    EXPECT_FALSE(map.contains(id2 + 1));

    map.erase(map.find(id2));

    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.contains(id1));
    EXPECT_FALSE(map.contains(id2));
}

}
