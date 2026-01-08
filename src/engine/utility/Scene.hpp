#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_SCENE_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_SCENE_HPP

#include "core/DescriptorPool.hpp"
#include "core/DescriptorSetLayout.hpp"
#include "core/Device.hpp"
#include "core/Texture2D.hpp"
#include "utility/Model.hpp"
#include "utility/material/DefaultTextureProvider.hpp"
#include "utility/material/Material.hpp"
#include "utility/object/Object.hpp"

#include "nlohmann/json-schema.hpp"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace nlohmann
{

template<>
struct adl_serializer<glm::vec3>
{
    static void to_json(json& j, const glm::vec3& v) { j = json::array({ v.x, v.y, v.z }); }

    static void from_json(const json& j, glm::vec3& v)
    {
        if(!j.is_array() || j.size() != 3)
        {
            // NOLINTNEXTLINE(readability-magic-numbers): defined by https://json.nlohmann.me/home/exceptions/#jsonexceptiontype_error302
            throw json::type_error::create(302, "expected array of size 3 for glm::vec3", &j);
        }

        j.at(0).get_to<float>(v.x);
        j.at(1).get_to<float>(v.y);
        j.at(2).get_to<float>(v.z);
    }
};

} // namespace nlohmann

namespace vv
{

/// \brief Helper class to define a custom schema violation error message format
///
/// \author Felix Hommel
/// \date 1/6/2026
class JsonValidatorErrorHandler : public nlohmann::json_schema::basic_error_handler
{
    void error(
        const nlohmann::json::json_pointer& ptr, const nlohmann::json& instance, const std::string& message
    ) override
    {
        nlohmann::json_schema::basic_error_handler::error(ptr, instance, message);
        spdlog::error("\'{}\' - \'{}\': {}", ptr.to_string(), instance.dump(), message);
    }
};

/// \brief A \ref Scene is a collection of lights and objects that all can contribute to the final rendered image
///
/// \author Felix Hommel
/// \date 1/8/2026
class Scene
{
public:
    using json = nlohmann::json;

    static constexpr auto JSON_MATERIALS_ACC{ "materials" };
    static constexpr auto JSON_TEXTURES_ACC{ "textures" };
    static constexpr auto JSON_ALBEDO_TEXTURE_ACC{ "albedo" };
    static constexpr auto JSON_NORMAL_TEXTURE_ACC{ "normal" };
    static constexpr auto JSON_METALLIC_ROUGHNESS_TEXTURE_ACC{ "metallicRoughness" };
    static constexpr auto JSON_OCCLUSION_TEXTURE_ACC{ "occlusion" };
    static constexpr auto JSON_EMISSION_TEXTURE_ACC{ "emission" };
    static constexpr auto JSON_HEIGHT_TEXTURE_ACC{ "height" };
    static constexpr auto JSON_MATERIAL_NAME_ACC{ "name" };
    static constexpr auto JSON_MODELS_ACC{ "models" };
    static constexpr auto JSON_MODEL_NAME_ACC{ "name" };
    static constexpr auto JSON_MODEL_PATH_ACC{ "path" };
    static constexpr auto JSON_OBJECS_ACC{ "objects" };
    static constexpr auto JSON_OBJECT_MODEL_ACC{ "model" };
    static constexpr auto JSON_OBJECT_MATERIAL_ACC{ "material" };
    static constexpr auto JSON_OBJECT_TRANSFORM_ACC{ "transform" };
    static constexpr auto JSON_OBJECT_TRANSFORM_POSITION_ACC{ "position" };
    static constexpr auto JSON_OBJECT_TRANSFORM_SCALE_ACC{ "scale" };
    static constexpr auto JSON_LIGHTS_ACC{ "lights" };
    static constexpr auto JSON_LIGHT_POINT_LIGHT_ACC{ "pointLight" };
    static constexpr auto JSON_LIGHT_POINT_LIGHT_INTENSITY_ACC{ "intensity" };
    static constexpr auto JSON_LIGHT_POINT_LIGHT_COLOR_ACC{ "color" };
    static constexpr auto JSON_LIGHT_POSITION_ACC{ "position" };

    /// \brief Construct a new Scene
    ///
    /// \param device a shared_ptr to the \ref Device which is used to allocate resources.
    /// \param materialLayout a shared_ptr to the \ref DescriptorSetLayout where PBR materials are allocated from
    explicit Scene(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> materialLayout);
    ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene(Scene&&) = default;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = default;

    [[nodiscard]] std::shared_ptr<Object::ObjectMap> getObjects() const { return m_objects; }
    [[nodiscard]] const std::vector<Object>& getPointLights() const { return m_pointLights; }
    [[nodiscard]] std::vector<Object>& getPointLights() { return m_pointLights; }

    [[nodiscard]] static Scene loadFromFile(
        const std::filesystem::path& filepath,
        std::shared_ptr<Device> device,
        std::shared_ptr<DescriptorSetLayout> materialLayout
    );
    [[nodiscard]] static Scene loadFromText(
        std::string_view jsonText, std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> materialLayout
    );

private:
    static constexpr std::size_t MAX_MATERIAL_SETS{ 100 };
    static constexpr std::size_t MATERIAL_POOL_SIZE{ 500 };
    static constexpr auto SCENE_JSON_SCHEMA_FILE{ PROJECT_ROOT "resources/scene-schema.json" };

    static Scene loadFromJson(
        const json& j, std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> materialLayout
    );
    static bool validateJson(const json& j);

    // Utility for managing resources of a scene
    std::shared_ptr<Device> m_device;                          ///< Used to allocate textures on
    std::shared_ptr<DefaultTextureProvider> m_defaultTextures; ///< Can use when a Material has no textures
    std::shared_ptr<DescriptorSetLayout> m_materialSetLayout;  ///< Descriptor set layout for materials
    std::unique_ptr<DescriptorPool> m_materialPool;            ///< Pool to allocate material descriptors from

    // Containers for scene resources
    std::unordered_map<std::filesystem::path, std::shared_ptr<Texture2D>> m_textureCache; ///< Textures
    std::unordered_map<std::string, std::shared_ptr<Material>> m_materialCache;           ///< Materials
    std::unordered_map<std::string, std::shared_ptr<Model>> m_modelCache;                 ///< Models

    // Scene objects
    std::shared_ptr<Object::ObjectMap> m_objects; ///< Objects
    std::vector<Object> m_pointLights;            ///< Lights

    void loadTexture(const std::filesystem::path& filepath, const TextureConfig& config);
    void loadMaterial(const std::string& materialName, MaterialConfig& materialConfig);
    void loadModel(const std::string& modelName, const std::filesystem::path& filepath);
    void loadObject(
        const std::string& modelName, const std::string& materialName, const glm::vec3& position, const glm::vec3& scale
    );
    void loadLight(float intensity, const glm::vec3& color, const glm::vec3& position);

    VkDescriptorSet allocateMaterialDescriptorSet(MaterialConfig& config);

    friend void from_json(const nlohmann::json& j, Scene& s);
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_SCENE_HPP

