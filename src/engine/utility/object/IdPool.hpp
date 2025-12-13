#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_ID_POOL_COMPONENT_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_ID_POOL_COMPONENT_HPP

#include <cstdint>
#include <limits>

namespace vv
{

using ObjectId_t = std::uint32_t; // NOTE: This means that currently there can be a max of 294967295 Objects
static constexpr auto INVALID_OBJECT_ID{ std::numeric_limits<ObjectId_t>::max() };

/// \brief The IdPool is used as an id provider for \ref Object.
/// It manages the freeing of id's of objects that have been destroyed which makes them available again
///
/// \author Felix Hommel
/// \date 12/7/2025
class IdPool
{
public:
    /// \brief Get an available id
    /// if there are any previously used ids free, the function prefers to assign them again
    ///
    /// \returns the next best free id
    ObjectId_t acquire() noexcept { return m_nextFreshId++; }

    /// \brief Add \p id to the list of ids that are free again
    ///
    /// \param id the id that became free
    // void release(ObjectId_t id) const
    //{
    //}

    /// \brief Reset the state of the pool
    void reset() noexcept { m_nextFreshId = 0; }

private:
    ObjectId_t m_nextFreshId{ std::numeric_limits<ObjectId_t>::min() };
    // TODO: For the future, probably should implement a system that can reacquire ids that are not used anymore
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_ID_POOL_COMPONENT_HPP
