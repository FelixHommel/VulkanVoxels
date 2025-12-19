#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_MATERIAL_MATERIAL_BUILDER_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_MATERIAL_MATERIAL_BUILDER_HPP

#include "core/DescriptorSetLayout.hpp"
#include "core/DescriptorPool.hpp"
#include "core/Device.hpp"
#include "core/Texture2D.hpp"
#include "utility/material/Material.hpp"
#include "utility/material/MaterialAlphaMode.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include <memory>

namespace vv
{

/// \brief Builder used to construct a new \ref Material
///
/// \author Felix Hommel
/// \date 12/17/2025
class MaterialBuilder
{
public:
    /// \brief Add a albedo texture to the material
    ///
    /// \param tex \ref Texture2D with the albedo image
    /// \param factor for scaling the color components
    MaterialBuilder& withAlbedo(std::shared_ptr<Texture2D> tex, const glm::vec4& factor = MaterialConfig::DEFAULT_BASE_COLOR_FACTOR);
    /// \brief Add a normal texture to the material
    ///
    /// \param tex \ref Texture2D with the normal image
    /// \param scale for the normal image
    MaterialBuilder& withNormal(std::shared_ptr<Texture2D> tex, float scale = MaterialConfig::DEFAULT_NORMAL_SCALE);
    /// \brief Add a metallic/roughness texture to the material
    ///
    /// \param tex \ref Texture2D with the metallic/roughness image
    /// \param factorMetallic for scaling the metallicness part of the texture
    /// \param factorRoughness for scaling the roughness part of the texture
    MaterialBuilder& withMetallicRoughness(std::shared_ptr<Texture2D> tex, float factorMetallic = MaterialConfig::DEFAULT_METALLIC_FACTOR, float factorRoughness = MaterialConfig::DEFAULT_ROUGHNESS_FACTOR);
    /// \brief Add a occlusiion texture to the image
    ///
    /// \param tex \ref Texture2D with the occlusion image
    /// \param strength how strong the occlusion effect is
    MaterialBuilder& withOcclusion(std::shared_ptr<Texture2D> tex, float strength = MaterialConfig::DEFAULT_OCCLUSION_STRENGTH);
    /// \brief Add a emission texture to the image
    ///
    /// \param tex \ref Texture2D with the emission image
    /// \param factor how string the emission is
    MaterialBuilder& withEmission(std::shared_ptr<Texture2D> tex, const glm::vec3& factor = MaterialConfig::DEFAULT_EMISSIVE_FACTOR);

    /// \brief Which \ref AlphaMode to use
    ///
    /// \param mode \ref AlphaMode to use
    MaterialBuilder& alphaMode(AlphaMode mode);
    /// \brief Configure a custom cutoff value
    ///
    /// \param cutoff the cutoff value
    MaterialBuilder& alphaCutoff(float cutoff);
    /// \brief Enable double sided rendering
    ///
    /// \param enable *true* if enabled, *false* if disabled
    MaterialBuilder& doubleSided(bool enable);

    /// \brief Build the configured material
    ///
    /// \param device \ref Device used for configuring layouts and push constants
    /// \param pool \ref DescriptorPool for setting up descriptors for the material
    /// \param layout \ref DescriptorSetLayout that is used
    ///
    /// \returns \ref Material constructed from the configuration
    Material build(std::shared_ptr<Device> device, std::shared_ptr<DescriptorPool> pool, std::shared_ptr<DescriptorSetLayout> layout);

private:
    MaterialConfig config{};
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_MATERIAL_MATERIAL_BUILDER_HPP

