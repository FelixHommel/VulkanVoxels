#include "ObjectBuilder.hpp"

#include "utility/Model.hpp"
#include "utility/object/Object.hpp"
#include "utility/object/components/ColorComponent.hpp"
#include "utility/object/components/ModelComponent.hpp"
#include "utility/object/components/PointLightComponent.hpp"
#include "utility/object/components/TransformComponent.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include <memory>
#include <utility>

namespace vv
{

ObjectBuilder& ObjectBuilder::withColor(const glm::vec3& color)
{
    m_color = std::make_unique<ColorComponent>(color);
    return *this;
}

ObjectBuilder& ObjectBuilder::withModel(std::shared_ptr<Model> model)
{
    m_model = std::make_unique<ModelComponent>(std::move(model));
    return *this;
}

ObjectBuilder& ObjectBuilder::withPointLight(float intensity, const glm::vec3& color, float radius)
{
    m_pointLight = std::make_unique<PointLightComponent>(intensity, color, radius);
    return *this;
}

ObjectBuilder& ObjectBuilder::withTransform(const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& rotation)
{
    m_transform = std::make_unique<TransformComponent>(translation, scale, rotation);
    return *this;
}

std::unique_ptr<Object> ObjectBuilder::buildUnique()
{
    auto obj{ std::make_unique<Object>() };

    if(m_color)
        obj->addComponent<ColorComponent>(std::move(m_color));
    if(m_model)
        obj->addComponent<ModelComponent>(std::move(m_model));
    if(m_pointLight)
        obj->addComponent<PointLightComponent>(std::move(m_pointLight));
    if(m_transform)
        obj->addComponent<TransformComponent>(std::move(m_transform));

    return obj;
}

Object ObjectBuilder::buildRaw()
{
    Object obj;

    if(m_color)
        obj.addComponent<ColorComponent>(std::move(m_color));
    if(m_model)
        obj.addComponent<ModelComponent>(std::move(m_model));
    if(m_pointLight)
        obj.addComponent<PointLightComponent>(std::move(m_pointLight));
    if(m_transform)
        obj.addComponent<TransformComponent>(std::move(m_transform));

    return obj;
}

} // namespace vv
