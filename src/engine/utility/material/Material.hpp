#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_MATERIAL_MATERIAL_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_MATERIAL_MATERIAL_HPP

#include "core/Device.hpp"
#include "core/Texture2D.hpp"
#include "utility/material/MaterialAlphaMode.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include <vulkan/vulkan_core.h>

#include <memory>

namespace vv
{

/// \brief Helper struct to more easily configure a texture
///
/// \author Felix Hommel
/// \date 12/17/2025
struct MaterialConfig
{
public:
    static constexpr auto DEFAULT_BASE_COLOR_FACTOR{ glm::vec4(1.f) };
    static constexpr auto DEFAULT_NORMAL_SCALE{ 1.f };
    static constexpr auto DEFAULT_METALLIC_FACTOR{ 1.f };
    static constexpr auto DEFAULT_ROUGHNESS_FACTOR{ 1.f };
    static constexpr auto DEFAULT_OCCLUSION_STRENGTH{ 1.f };
    static constexpr auto DEFAULT_EMISSIVE_FACTOR{ glm::vec3(0.f) };
    static constexpr auto DEFAULT_ALPHA_CUTOFF{ 1.f };
    static constexpr auto DEFAULT_ALPHA_MODE{ AlphaMode::Opaque };
    static constexpr auto DEFAULT_DOUBLE_SIDED{ false };

    std::shared_ptr<Texture2D> albedoTexture;
    std::shared_ptr<Texture2D> normalTexture;
    std::shared_ptr<Texture2D> metallicRoughnessTexture;
    std::shared_ptr<Texture2D> occlusionTexture;
    std::shared_ptr<Texture2D> emissiveTexture;

    glm::vec4 baseColorFactor{ DEFAULT_BASE_COLOR_FACTOR };
    float normalScale{ DEFAULT_NORMAL_SCALE };
    float metallicFactor{ DEFAULT_METALLIC_FACTOR };
    float roughnessFactor{ DEFAULT_ROUGHNESS_FACTOR };
    float occlusionStrength{ DEFAULT_OCCLUSION_STRENGTH };
    glm::vec3 emissiveFactor{ DEFAULT_EMISSIVE_FACTOR };
    float alphaCutoff{ DEFAULT_ALPHA_CUTOFF };
    AlphaMode alphaMode{ DEFAULT_ALPHA_MODE };
    bool doubleSided{ DEFAULT_DOUBLE_SIDED };
};

static constexpr unsigned int PUSH_VECTOR_ALIGNMENT{ 16 };

/// \brief Push constant layout for materials
///
/// \author Felix Hommel
/// \date 12/17/2025
struct MaterialPushConstants
{
public:
    glm::vec4 baseColorFactor;
    alignas(PUSH_VECTOR_ALIGNMENT) glm::vec3 emissiveFactor;
    float normalScale;
    float metallicFactor;
    float roughnessFactor;
    float occlusionStrength;
    float alphaCutoff;
};

/// \brief PBR based Material
///
/// \author Felix Hommel
/// \date 12/16/2025
class Material
{
public:
    /// \brief Create a new Material
    Material(std::shared_ptr<Device> device, const MaterialConfig& config, VkDescriptorSet descriptor);
    Material(Material&& other) noexcept;
    ~Material() = default;

    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;
    Material& operator=(Material&&) = delete;

    /// \brief Binde the material to the pipeline
    ///
    /// This does not bind any models or meshes - only the textures related to the material are bound by this method
    ///
    /// \param commandBuffer the command buffer that is rendered to
    /// \param layoput the layout of the used graphics pipeline
    void bind(VkCommandBuffer commandBuffer, VkPipelineLayout layout);

private:
    std::shared_ptr<Device> device;

    // Textures (optional)
    std::shared_ptr<Texture2D> m_albedoTexture;            ///< Base color of the material
    std::shared_ptr<Texture2D> m_normalTexture;            ///< Surface details (no geometry required)
    std::shared_ptr<Texture2D> m_metallicRoughnessTexture; ///< How metalloc and how rough the material is
    std::shared_ptr<Texture2D> m_occlusionTexture;         ///< Ambient shadow in gaps
    std::shared_ptr<Texture2D> m_emissiveTexture;          ///< Self illumination and light emission

    // Factors (always present)
    glm::vec4 m_baseColorFactor; ///< Tint the albedo texture
    float m_normalScale;         ///< Scaling factor for normals
    float m_metallicFactor;      ///< Scaling factor for metallic
    float m_roughnessFactor;     ///< Scaling factor for roughness
    float m_occlusionStrength;   ///< Scaling factor for occlusion
    glm::vec3 m_emissiveFactor;  ///< Scaling factor for emission

    // Rendering state
    AlphaMode m_alphaMode; ///< Alpha property of the Material
    float m_alphaCutoff;   ///< When a Material is fully non-seethrough
    bool m_doubleSided;    ///< Whether to render both sides or not

    // Vulkan Resources
    VkDescriptorSet m_descriptorSet{ VK_NULL_HANDLE }; ///< The Descriptor set for the Material
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_MATERIAL_MATERIAL_HPP

