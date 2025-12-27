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

#include <cstddef>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

namespace vv
{

class Scene
{
public:
    Scene(std::shared_ptr<Device> device, std::shared_ptr<DescriptorSetLayout> materialLayout);
    ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene(Scene&&) = default;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = default;

    void createMaterial(const MaterialConfig& config);
    void addObject(Object&& o);
    void addPointlight(const glm::vec3& position, const glm::vec3& color, float intensity);

    [[nodiscard]] std::shared_ptr<Object::ObjectMap> getObjects() const { return m_objects; }
    [[nodiscard]] const std::vector<Object>& getPointLights() const { return m_pointLights; }

private:
    static constexpr std::size_t MAX_MATERIAL_SETS{ 100 };
    static constexpr std::size_t MATERIAL_POOL_SIZE{ 500 };

    // Utility for managing resources of a scene
    std::shared_ptr<Device> m_device;                          ///< Used to allocate textures on
    std::shared_ptr<DefaultTextureProvider> m_defaultTextures; ///< Can use when a Material has no textures
    std::shared_ptr<DescriptorSetLayout> m_materialSetLayout;  ///< Descriptor set layout for materials
    std::unique_ptr<DescriptorPool> m_materialPool;            ///< Pool to allocate material descriptors from

    // Containers for scene resources
    std::unordered_map<std::filesystem::path, std::shared_ptr<Texture2D>> m_textureCache; ///< Textures
    std::unordered_map<std::filesystem::path, std::shared_ptr<Model>> m_modelCache;       ///< Models
    std::vector<std::shared_ptr<Material>> m_materialCache;                               ///< Materials

    // Scene objects
    std::shared_ptr<Object::ObjectMap> m_objects; ///< Objects
    std::vector<Object> m_pointLights;            ///< Lights

    VkDescriptorSet allocateMaterialDescriptorSet(const MaterialConfig& config);
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_SCENE_HPP

