#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_UTILS_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_UTILS_HPP

#include <cstddef>
#include <functional>

namespace vv
{

// See https://stackoverflow.com/a/57595105 for more details
template <typename T, typename... Rest> void hashCombine(std::size_t& seed, const T& v, const Rest&... rest)
{
    // NOLINTNEXTLINE(readability-magic-numbers): It is a hash function, so magic numbers are tolerated
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
}

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_UTILS_HPP
