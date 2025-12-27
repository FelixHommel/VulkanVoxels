#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_MATERIAL_COMPONENT_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_MATERIAL_COMPONENT_HPP

#include "utility/material/Material.hpp"
#include "utility/object/components/Component.hpp"
#include <memory>
namespace vv
{

/// \brief Add a Material to an \ref Object
///
/// \author Felix Hommel
/// \date 12/27/2025
struct MaterialComponent final : public Component
{
    explicit MaterialComponent(std::shared_ptr<Material> material) : material(std::move(material)) {}

    std::shared_ptr<Material> material;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_MATERIAL_COMPONENT_HPP

