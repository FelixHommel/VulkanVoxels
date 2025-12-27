#include "Material.hpp"

#include "core/Device.hpp"
#include "renderSystems/IRenderSystem.hpp"

#include <vulkan/vulkan_core.h>

#include <memory>
#include <utility>

namespace vv
{

Material::Material(std::shared_ptr<Device> device, const MaterialConfig& config, VkDescriptorSet descriptor)
    : device(std::move(device))
    , m_albedoTexture(std::move(config.albedoTexture))
    , m_normalTexture(std::move(config.normalTexture))
    , m_metallicRoughnessTexture(std::move(config.metallicRoughnessTexture))
    , m_occlusionTexture(std::move(config.occlusionTexture))
    , m_emissiveTexture(std::move(config.emissiveTexture))
    , m_baseColorFactor{ config.baseColorFactor }
    , m_normalScale{ config.normalScale }
    , m_metallicFactor{ config.metallicFactor }
    , m_roughnessFactor{ config.roughnessFactor }
    , m_occlusionStrength{ config.occlusionStrength }
    , m_emissiveFactor{ config.emissiveFactor }
    , m_alphaCutoff{ config.alphaCutoff }
    , m_alphaMode{ config.alphaMode }
    , m_doubleSided{ config.doubleSided }
    , m_descriptorSet(descriptor)
{}

Material::Material(Material&& other) noexcept
    : device(std::move(other.device))
    , m_albedoTexture(std::move(other.m_albedoTexture))
    , m_normalTexture(std::move(other.m_normalTexture))
    , m_metallicRoughnessTexture(std::move(other.m_metallicRoughnessTexture))
    , m_occlusionTexture(std::move(other.m_occlusionTexture))
    , m_emissiveTexture(std::move(other.m_emissiveTexture))
    , m_baseColorFactor{ other.m_baseColorFactor }
    , m_normalScale{ other.m_normalScale }
    , m_metallicFactor{ other.m_metallicFactor }
    , m_roughnessFactor{ other.m_roughnessFactor }
    , m_occlusionStrength{ other.m_occlusionStrength }
    , m_emissiveFactor{ other.m_emissiveFactor }
    , m_alphaCutoff{ other.m_alphaCutoff }
    , m_alphaMode{ other.m_alphaMode }
    , m_doubleSided{ other.m_doubleSided }
    , m_descriptorSet(other.m_descriptorSet)
{}

void Material::bind(VkCommandBuffer commandBuffer, VkPipelineLayout layout)
{
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1, &m_descriptorSet, 0, nullptr);

    MaterialPushConstants push{
        .baseColorFactor = m_baseColorFactor,
        .emissiveFactor = m_emissiveFactor,
        .normalScale = m_normalScale,
        .metallicFactor = m_metallicFactor,
        .roughnessFactor = m_roughnessFactor,
        .occlusionStrength = m_occlusionStrength,
    };

    vkCmdPushConstants(
        commandBuffer,
        layout,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        sizeof(SimplePushConstantData),
        sizeof(MaterialPushConstants),
        &push
    );
}

} // namespace vv

