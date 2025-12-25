#include "MaterialBuilder.hpp"

#include "core/DescriptorPool.hpp"
#include "core/DescriptorSetLayout.hpp"
#include "core/Device.hpp"
#include "core/Texture2D.hpp"
#include "utility/material/Material.hpp"
#include "utility/material/MaterialAlphaMode.hpp"

#include <memory>
#include <utility>

namespace vv
{

MaterialBuilder& MaterialBuilder::withAlbedo(std::shared_ptr<Texture2D> tex, const glm::vec4& factor)
{
    config.albedoTexture = std::move(tex);
    config.baseColorFactor = factor;

    return *this;
}

MaterialBuilder& MaterialBuilder::withNormal(std::shared_ptr<Texture2D> tex, float scale)
{
    config.normalTexture = std::move(tex);
    config.normalScale = scale;

    return *this;
}

MaterialBuilder& MaterialBuilder::withMetallicRoughness(
    std::shared_ptr<Texture2D> tex, float factorMetallic, float factorRoughness
)
{
    config.metallicRoughnessTexture = std::move(tex);
    config.metallicFactor = factorMetallic;
    config.roughnessFactor = factorRoughness;

    return *this;
}

MaterialBuilder& MaterialBuilder::withOcclusion(std::shared_ptr<Texture2D> tex, float strength)
{
    config.occlusionTexture = std::move(tex);
    config.occlusionStrength = strength;

    return *this;
}

MaterialBuilder& MaterialBuilder::withEmission(std::shared_ptr<Texture2D> tex, const glm::vec3& factor)
{
    config.emissiveTexture = std::move(tex);
    config.emissiveFactor = factor;

    return *this;
}

MaterialBuilder& MaterialBuilder::alphaMode(AlphaMode mode)
{
    config.alphaMode = mode;

    return *this;
}

MaterialBuilder& MaterialBuilder::alphaCutoff(float cutoff)
{
    config.alphaCutoff = cutoff;

    return *this;
}

MaterialBuilder& MaterialBuilder::doubleSided(bool enable)
{
    config.doubleSided = enable;

    return *this;
}

Material MaterialBuilder::build(
    std::shared_ptr<Device> device,
    std::shared_ptr<DescriptorPool> /*pool*/,
    std::shared_ptr<DescriptorSetLayout> /*layout*/
)
{
    return { std::move(device), config };
}

} // namespace vv

