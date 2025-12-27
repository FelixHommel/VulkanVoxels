#include "Scene.hpp"

#include "core/DescriptorWriter.hpp"
#include "core/Device.hpp"
#include "utility/exceptions/Exception.hpp"
#include "utility/material/DefaultTextureProvider.hpp"

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

void Scene::createMaterial(const MaterialConfig& config)
{
    VkDescriptorSet descriptorSet{ allocateMaterialDescriptorSet(config) };
    m_materialCache.emplace_back(std::make_shared<Material>(m_device, config, descriptorSet));
}

void Scene::addObject(Object&& o)
{
    m_objects->emplace(o.getId(), std::move(o));
}

void Scene::addPointlight(const glm::vec3& position, const glm::vec3& color, float intensity) {}

VkDescriptorSet Scene::allocateMaterialDescriptorSet(const MaterialConfig& config)
{
    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };

    if(!m_materialPool->allocateDescriptor(m_materialSetLayout->getDescriptorLayout(), descriptorSet))
        throw Exception("Failed to allocate material descriptor set");

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


