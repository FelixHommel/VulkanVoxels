#ifndef SRC_ENGINE_MAGNET_PHYSICS_SYSTEM_HPP
#define SRC_ENGINE_MAGNET_PHYSICS_SYSTEM_HPP

#include "Object.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <vector>
#include <cmath>

namespace vv
{

class MagnetPhysicsSystem
{
public:
    const float gravityStrength;

    MagnetPhysicsSystem(float strength) : gravityStrength(strength) {}
    ~MagnetPhysicsSystem() = default;

    MagnetPhysicsSystem(const MagnetPhysicsSystem&) = delete;
    MagnetPhysicsSystem(MagnetPhysicsSystem&&) = delete;
    MagnetPhysicsSystem& operator=(const MagnetPhysicsSystem&) = delete;
    MagnetPhysicsSystem& operator=(MagnetPhysicsSystem&&) = delete;

    void update(std::vector<Object>& objs, float dt, unsigned int substeps = 1)
    {
        const float dStep{ dt / substeps };
        for(unsigned int i{ 0 }; i < substeps; ++i)
            stepSimulation(objs, dStep);
    }

    glm::vec2 computeForce(const Object& from, const Object& to) const
    {
        auto offset{ from.transform2d.translation - to.transform2d.translation };
        float distanceSquared{ glm::dot(offset, offset) };

        if(std::abs(distanceSquared) < 1e-10f)
            return { 0.f, 0.f };

        float force{ gravityStrength * to.rigidBody2d.mass * from.rigidBody2d.mass / distanceSquared };

        return force * offset / std::sqrt(distanceSquared);
    }

private:
    void stepSimulation(std::vector<Object>& physicsObjs, float dt)
    {
        for(auto itA{ physicsObjs.begin() }; itA != physicsObjs.end(); ++itA)
        {
            auto& objA{ *itA };

            for(auto itB{ itA }; itB != physicsObjs.end(); ++itB)
            {
                if(itA == itB)
                    continue;

                auto& objB{ *itB };

                auto force{ computeForce(objA, objB) };
                objA.rigidBody2d.velocity += dt * -force / objA.rigidBody2d.mass;
                objB.rigidBody2d.velocity += dt * -force / objB.rigidBody2d.mass;
            }
        }

        for(auto& obj : physicsObjs)
            obj.transform2d.translation += dt * obj.rigidBody2d.velocity;
    }
};

class Vec2FieldSystem
{
public:
    void update(const MagnetPhysicsSystem& physicsSystem, const std::vector<Object>& physicsObjs, std::vector<Object>& vectorField)
    {
        for(auto& vf : vectorField)
        {
            glm::vec2 direction{};

            for(auto& obj : physicsObjs)
                direction += physicsSystem.computeForce(obj, vf);

            vf.transform2d.scale.x = 0.005f + 0.045f * glm::clamp(glm::log(glm::length(direction) + 1) / 3.f, 0.f, 1.f);
            vf.transform2d.rotation = std::atan2(direction.y, direction.x);
        }
    }
};

} //!vv

#endif //!SRC_ENGINE_MAGNET_PHYSICS_SYSTEM_HPP
