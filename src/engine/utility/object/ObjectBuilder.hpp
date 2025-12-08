#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_OBJECT_BUILDER_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_OBJECT_BUILDER_HPP

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

namespace vv
{

/// \brief Builder which helps to construct new \ref Object objects
///
/// \author Felix Hommel
/// \date 12/7/2025
class ObjectBuilder
{
public:
    /// \brief Add a color component to the \ref Object
    ///
    /// \param color which color the \ref Object is going to have
    ObjectBuilder& withColor(const glm::vec3& color);
    /// \brief Add a Model component, giving the object a mesh representation
    ///
    /// \param model shared pointer to the model \ref Object
    ObjectBuilder& withModel(std::shared_ptr<Model> model);
    /// \brief Add a point light component to the \ref Object
    ///
    /// \param intensity (optional) how strong the light is
    /// \param radius (optional) how big the light is
    /// \param color (optional) which color the light has
    ObjectBuilder& withPointLight(float intensity = PointLightComponent::DEFAULT_INTENSITY, const glm::vec3& color = PointLightComponent::DEFAULT_COLOR, float radius = PointLightComponent::DEFAULT_RADIUS);
    /// \brief Add a transform component to the \ref Object
    ///
    /// \param translation (optional) starting offset of the \ref Object
    /// \param scale (optional) scaling of the \ref Object
    /// \param rotation (optional) rotation of the \ref Object
    ObjectBuilder& withTransform(const glm::vec3& translation = {}, const glm::vec3& scale = glm::vec3(1.f), const glm::vec3& rotation = {});
    /// \brief Build the \ref Object as unique_ptr
    ///
    /// \returns unique_ptr to the \ref Object
    std::unique_ptr<Object> buildUnique();
    /// \brief Build the \ref Object as regular object
    ///
    /// \returns newly created \ref Object
    Object buildRaw();

private:
    std::unique_ptr<ColorComponent> m_color;
    std::unique_ptr<ModelComponent> m_model;
    std::unique_ptr<PointLightComponent> m_pointLight;
    std::unique_ptr<TransformComponent> m_transform;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_OBJECT_BUILDER_HPP
