#include "Scene.hpp"

#include "core/DescriptorWriter.hpp"
#include "core/Device.hpp"
#include "utility/exceptions/Exception.hpp"
#include "utility/material/DefaultTextureProvider.hpp"
#include "utility/object/ObjectBuilder.hpp"

#include <memory>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace vv
{

Scene::Scene(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> materialLayout)
    : m_device(std::move(device))
    , m_defaultTextures{ std::make_shared<DefaultTextureProvider>(this->m_device) }
    , m_materialSetLayout(std::move(materialLayout))
    , m_objects{ std::make_shared<Object::ObjectMap>() }
{
    m_materialPool = DescriptorPool::Builder(m_device)
                         .setMaxSets(MAX_MATERIAL_SETS)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MATERIAL_POOL_SIZE)
                         .build();
}

std::shared_ptr<Material> Scene::createMaterial(MaterialConfig& config)
{
    VkDescriptorSet descriptorSet{ allocateMaterialDescriptorSet(config) };
    m_materialCache.emplace_back(std::make_shared<Material>(m_device, config, descriptorSet));

    return m_materialCache.back();
}

void Scene::addObject(Object&& o)
{
    m_objects->emplace(o.getId(), std::move(o));
}

void Scene::addPointlight(Object&& o)
{
    m_pointLights.emplace_back(std::move(o));
}

VkDescriptorSet Scene::allocateMaterialDescriptorSet(MaterialConfig& config)
{
    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

    if(!m_materialPool->allocateDescriptor(m_materialSetLayout->getDescriptorLayout(), descriptorSet))
        throw Exception("Failed to allocate material descriptor set");

    if(!config.albedoTexture)
        config.albedoTexture = m_defaultTextures->white();
    if(!config.normalTexture)
        config.normalTexture = m_defaultTextures->normal();
    if(!config.metallicRoughnessTexture)
        config.metallicRoughnessTexture = m_defaultTextures->white();
    if(!config.occlusionTexture)
        config.occlusionTexture = m_defaultTextures->white();
    if(!config.emissiveTexture)
        config.emissiveTexture = m_defaultTextures->black();

    auto albedoInfo{ config.albedoTexture->descriptor() };
    auto normalInfo{ config.normalTexture->descriptor() };
    auto metallicRoughnessInfo{ config.metallicRoughnessTexture->descriptor() };
    auto occlusionInfo{ config.occlusionTexture->descriptor() };
    auto emissiveInfo{ config.emissiveTexture->descriptor() };

    DescriptorWriter(m_materialSetLayout.get(), m_materialPool.get())
        .writeImage(0, &albedoInfo)
        .writeImage(1, &normalInfo)
        .writeImage(2, &metallicRoughnessInfo)
        .writeImage(3, &occlusionInfo)
        .writeImage(4, &emissiveInfo)
        .overwrite(descriptorSet);

    return descriptorSet;
}

} // namespace vv


