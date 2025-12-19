#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_MATERIAL_MATERIAL_ALPHA_MODE_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_MATERIAL_MATERIAL_ALPHA_MODE_HPP

#include <cstdint>

namespace vv
{

/// \brief What kind of alpha behavior a material exhibits
///
/// \author Felix Hommel
/// \date 12/16/2025
enum class AlphaMode : std::uint8_t
{
    Opaque,
    Mask,
    Blend
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_MATERIAL_MATERIAL_ALPHA_MODE_HPP

