#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_COMPONENT_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_COMPONENT_HPP

namespace vv
{

/// \brief Base class for components, used to provide a common interface to ensure type safety for templated methods
///
/// \author Felix Hommel
/// \date 12/7/2025
struct Component
{
    Component() = default;
    virtual ~Component() = default;

    Component(const Component&) = default;
    Component(Component&&) = default;
    Component& operator=(const Component&) = default;
    Component& operator=(Component&&) = default;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_COMPONENTS_COMPONENT_HPP
