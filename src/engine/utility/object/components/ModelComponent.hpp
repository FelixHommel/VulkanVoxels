#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_MESH_COMPONENT_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_MESH_COMPONENT_HPP

#include "utility/Model.hpp"
#include "utility/object/components/Component.hpp"

#include <memory>
#include <utility>

namespace vv
{

/// \brief Component that gives an \ref Object the ability to have a representation by a mesh
///
/// \author Felix Hommel
/// \date 12/7/2025
struct ModelComponent final : public Component
{
    constexpr explicit ModelComponent(std::shared_ptr<Model> model) : model(std::move(model)) {}

    std::shared_ptr<Model> model;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_MESH_COMPONENT_HPP