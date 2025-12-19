#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_MATERIAL_DEFAULT_MATERIAL_PROVIDER_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_MATERIAL_DEFAULT_MATERIAL_PROVIDER_HPP

#include "core/Device.hpp"
#include "core/Texture2D.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <span>

namespace vv
{

/// \brief Provide a set of default textures for materials that don't have/need a certain type of texture
///
/// \author Felix Hommel
/// \date 12/18/2025
class DefaultTextureProvider
{
public:
    DefaultTextureProvider(std::shared_ptr<Device> device)
        : m_white{ std::make_shared<Texture2D>(device, 1, 1, TextureConfig::albedo(), std::span<const std::byte>(whiteColorArray)) }
        , m_black{ std::make_shared<Texture2D>(device, 1, 1, TextureConfig::albedo(), std::span<const std::byte>(blackColorArray)) }
        , m_normal{ std::make_shared<Texture2D>(device, 1, 1, TextureConfig::normal(), std::span<const std::byte>(normalColorArray)) }
    {
    }
    ~DefaultTextureProvider() = default;

    DefaultTextureProvider(const DefaultTextureProvider&) = delete;
    DefaultTextureProvider(DefaultTextureProvider&&) = delete;
    DefaultTextureProvider& operator=(const DefaultTextureProvider&) = delete;
    DefaultTextureProvider& operator=(DefaultTextureProvider&&) = delete;

    [[nodiscard]] std::shared_ptr<Texture2D> white() const noexcept { return m_white; }
    [[nodiscard]] std::shared_ptr<Texture2D> black() const noexcept { return m_black; }
    [[nodiscard]] std::shared_ptr<Texture2D> normal() const noexcept { return m_normal; }

private:
    static constexpr std::byte WHITE{ 255 };
    static constexpr std::array<std::byte, 4> whiteColorArray{ WHITE, WHITE, WHITE, WHITE };
    static constexpr std::byte BLACK{ 0 };
    static constexpr std::array<std::byte, 4> blackColorArray{ BLACK, BLACK, BLACK, BLACK };
    static constexpr std::byte HALF{ 128 };
    static constexpr std::array<std::byte, 4> normalColorArray{ HALF, HALF, HALF, WHITE };

    std::shared_ptr<Texture2D> m_white;
    std::shared_ptr<Texture2D> m_black;
    std::shared_ptr<Texture2D> m_normal;
};


} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_MATERIAL_DEFAULT_MATERIAL_PROVIDER_HPP

