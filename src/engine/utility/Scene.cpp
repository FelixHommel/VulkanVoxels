#include "Scene.hpp"

#include "core/DescriptorWriter.hpp"
#include "core/Device.hpp"
#include "core/Texture2D.hpp"
#include "utility/exceptions/Exception.hpp"
#include "utility/exceptions/FileException.hpp"
#include "utility/material/DefaultTextureProvider.hpp"
#include "utility/object/ObjectBuilder.hpp"

#include "nlohmann/json-schema.hpp"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include <string_view>
#include <vulkan/vulkan_core.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <utility>

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

Scene Scene::loadFromFile(
    const std::filesystem::path& filepath,
    std::shared_ptr<Device> device,
    std::shared_ptr<DescriptorSetLayout> materialLayout
)
{
    json scene{};

    try
    {
        std::ifstream sceneFile{ filepath };
        scene = json::parse(sceneFile);
        sceneFile.close();
    }
    catch(json::parse_error& e)
    {
        std::stringstream ss;
        ss << "Failed to parse json file:\n" << e.what();
        throw FileException(ss.str(), filepath);
    }

    return loadFromJson(scene, std::move(device), std::move(materialLayout));
}

Scene Scene::loadFromText(
    std::string_view jsonText, std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> materialLayout
)
{
    json scene{};

    try
    {
        scene = json::parse(jsonText);
    }
    catch(json::parse_error& e)
    {
        std::stringstream ss;
        ss << "Failed to parse json:\n" << e.what();
        throw Exception(ss.str());
    }

    return loadFromJson(scene, std::move(device), std::move(materialLayout));
}

void Scene::loadTexture(const std::filesystem::path& filepath, const TextureConfig& config)
{
    std::filesystem::path fullPath{ PROJECT_ROOT };
    fullPath /= filepath;
    spdlog::info("registering new texture: {}", fullPath.string());

    m_textureCache.emplace(filepath, std::make_shared<Texture2D>(Texture2D::loadFromFile(m_device, fullPath, config)));
}

void Scene::loadMaterial(const std::string& materialName, MaterialConfig& materialConfig)
{
    spdlog::info("registering new material: {}", materialName);

    VkDescriptorSet descriptorSet{ allocateMaterialDescriptorSet(materialConfig) };
    m_materialCache.emplace(materialName, std::make_shared<Material>(m_device, materialConfig, descriptorSet));
}

void Scene::loadModel(const std::string& modelName, const std::filesystem::path& filepath)
{
    spdlog::info("registering new model: {}", modelName);

    std::filesystem::path fullPath{ PROJECT_ROOT };
    fullPath /= filepath;

    m_modelCache.emplace(modelName, std::make_shared<Model>(Model::loadFromFile(m_device, fullPath)));
}

void Scene::loadObject(
    const std::string& modelName, const std::string& materialName, const glm::vec3& position, const glm::vec3& scale
)
{
    spdlog::info("registering new object (model: {}; material: {})", modelName, materialName);

    auto obj{ ObjectBuilder()
                  .withModel(m_modelCache.at(modelName))
                  .withMaterial(m_materialCache.at(materialName))
                  .withTransform(position, scale)
                  .build() };
    m_objects->emplace(obj.getId(), std::move(obj));
}

void Scene::loadLight(float intensity, const glm::vec3& color, const glm::vec3& position)
{
    spdlog::info("registering new light");

    m_pointLights.emplace_back(ObjectBuilder().withPointLight(intensity, color).withTransform(position).build());
}

Scene Scene::loadFromJson(
    const json& j, std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> materialLayout
)
{
    try
    {
        validateJson(j);
    }
    catch(json::parse_error& e)
    {
        throw Exception("Scene descriptor file didn't pass validation");
    }


    Scene s{ std::move(device), std::move(materialLayout) };
    j.get_to(s);

    return std::move(s);
}

bool Scene::validateJson(const json& j)
{
    nlohmann::json_schema::json_validator validator{};

    try
    {
        std::ifstream schemaFile{ std::filesystem::path(SCENE_JSON_SCHEMA_FILE) };
        const json schema{ json::parse(schemaFile) };
        schemaFile.close();

        validator.set_root_schema(schema);
    }
    catch(json::parse_error& e)
    {
        std::stringstream ss;
        ss << "Failed to parse scene descriptor schema file:\n" << e.what();
        throw FileException(ss.str(), SCENE_JSON_SCHEMA_FILE);
    }
    catch(std::exception& e)
    {
        std::stringstream ss;
        ss << "Validation of schema failed:\n" << e.what();
        throw FileException(ss.str(), SCENE_JSON_SCHEMA_FILE);
    }

    JsonValidatorErrorHandler err{};
    validator.validate(j, err);

    return !err;
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

void from_json(const nlohmann::json& j, Scene& s)
{
    for(const auto& material : j.at(Scene::JSON_MATERIALS_ACC))
    {
        MaterialConfig matConfig{};

        // NOTE: Step 1: Load all provided textures
        const auto& textures{ material.at(Scene::JSON_TEXTURES_ACC) };
        if(textures.at(Scene::JSON_ALBEDO_TEXTURE_ACC) != "")
        {
            s.loadTexture(
                textures.at(Scene::JSON_ALBEDO_TEXTURE_ACC).get<std::filesystem::path>(), TextureConfig::albedo()
            );
            matConfig.albedoTexture = s.m_textureCache.at(textures.at(Scene::JSON_ALBEDO_TEXTURE_ACC));
        }
        if(textures.at(Scene::JSON_NORMAL_TEXTURE_ACC) != "")
        {
            s.loadTexture(
                textures.at(Scene::JSON_NORMAL_TEXTURE_ACC).get<std::filesystem::path>(), TextureConfig::normal()
            );
            matConfig.normalTexture = s.m_textureCache.at(textures.at(Scene::JSON_NORMAL_TEXTURE_ACC));
        }
        if(textures.at(Scene::JSON_METALLIC_ROUGHNESS_TEXTURE_ACC) != "")
        {
            s.loadTexture(
                textures.at(Scene::JSON_METALLIC_ROUGHNESS_TEXTURE_ACC).get<std::filesystem::path>(),
                TextureConfig::metallicRoughness()
            );
            matConfig.metallicRoughnessTexture
                = s.m_textureCache.at(textures.at(Scene::JSON_METALLIC_ROUGHNESS_TEXTURE_ACC));
        }
        if(textures.at(Scene::JSON_OCCLUSION_TEXTURE_ACC) != "")
        {
            s.loadTexture(
                textures.at(Scene::JSON_OCCLUSION_TEXTURE_ACC).get<std::filesystem::path>(), TextureConfig::occlusion()
            );
            matConfig.occlusionTexture = s.m_textureCache.at(textures.at(Scene::JSON_OCCLUSION_TEXTURE_ACC));
        }
        if(textures.at(Scene::JSON_EMISSION_TEXTURE_ACC) != "")
        {
            s.loadTexture(
                textures.at(Scene::JSON_EMISSION_TEXTURE_ACC).get<std::filesystem::path>(), TextureConfig::emission()
            );
            matConfig.emissiveTexture = s.m_textureCache.at(textures.at(Scene::JSON_EMISSION_TEXTURE_ACC));
        }
        if(textures.at(Scene::JSON_HEIGHT_TEXTURE_ACC) != "")
        {
            // TODO: Uncomment once height maps are supported
            // s.loadTexture(textures.at(Scene::JSON_HEIGHT_TEXTURE_ACC).get<std::filesystem::path>(), TextureConfig::height());
            // matConfig.heightTexture = s.m_textureCache.at(textures.at(Scene::JSON_HEIGHT_TEXTURE_ACC));
        }

        // NOTE: Step 2: Create the material
        s.loadMaterial(material.at(Scene::JSON_MATERIAL_NAME_ACC).get<std::string>(), matConfig);
    }

    for(const auto& model : j.at(Scene::JSON_MODELS_ACC))
        s.loadModel(
            model.at(Scene::JSON_MODEL_NAME_ACC).get<std::string>(),
            model.at(Scene::JSON_MODEL_PATH_ACC).get<std::filesystem::path>()
        );

    for(const auto& object : j.at(Scene::JSON_OBJECS_ACC))
    {
        const auto& transform{ object.at(Scene::JSON_OBJECT_TRANSFORM_ACC) };

        s.loadObject(
            object.at(Scene::JSON_OBJECT_MODEL_ACC).get<std::string>(),
            object.at(Scene::JSON_OBJECT_MATERIAL_ACC).get<std::string>(),
            transform.at(Scene::JSON_OBJECT_TRANSFORM_POSITION_ACC).get<glm::vec3>(),
            transform.at(Scene::JSON_OBJECT_TRANSFORM_SCALE_ACC).get<glm::vec3>()
        );
    }

    for(const auto& light : j.at(Scene::JSON_LIGHTS_ACC))
    {
        const auto& pointLight{ light.at(Scene::JSON_LIGHT_POINT_LIGHT_ACC) };
        s.loadLight(
            pointLight.at(Scene::JSON_LIGHT_POINT_LIGHT_INTENSITY_ACC).get<float>(),
            pointLight.at(Scene::JSON_LIGHT_POINT_LIGHT_COLOR_ACC).get<glm::vec3>(),
            light.at(Scene::JSON_LIGHT_POSITION_ACC).get<glm::vec3>()
        );
    }
}

} // namespace vv


