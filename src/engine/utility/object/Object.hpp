#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_OBJECT_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_OBJECT_HPP

#include "utility/object/IdPool.hpp"

#include "components/Component.hpp"

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace vv
{

/// \brief An Object is used to represent arbitrary vertex data as objects and associate positions and other properties with them
///
/// \author Felix Hommel
/// \date 11/19/2025
class Object
{
public:
    using ObjectMap = std::unordered_map<ObjectId_t, Object>;

    /// \brief Construct a new \ref Object
    ///
    /// Manages automatic id management for objects
    Object();
    ~Object() = default;

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&& other) noexcept;
    Object& operator=(Object&& other) noexcept;

    [[nodiscard]] ObjectId_t getId() const noexcept { return m_id; }

    /// \brief Check if a component has already been registered
    ///
    /// \tparam T any of the types that inherit from \ref Component
    ///
    /// \returns *true* if T has been registered, *false* if not
    template<typename T>
        requires std::is_base_of_v<Component, T>
    bool hasComponent() const
    {
        return m_components.contains(typeid(T));
    }

    /// \brief Add a new component to the object
    ///
    /// \tparam T type of the Component. Must inherit from \ref Component
    ///
    /// \param component the actual component
    template<typename T>
        requires std::is_base_of_v<Component, T>
    void addComponent(std::unique_ptr<T> component)
    {
        // NOTE: currently is silent-fail. May change to a more expressive solution in the future (i.e., exception)
        if(hasComponent<T>())
            return;

        m_components.emplace(typeid(T), std::move(component));
    }

    /// \brief Add a new component to the object
    /// This will automatically construct the new component with the provided arguments
    ///
    /// \tparam T type of the Component. Must inherit from \ref Component
    /// \tparam Args type of the arguments required to construct T
    /// \param args arguments that are passed to the T constructor
    template<typename T, typename... Args>
        requires std::is_base_of_v<Component, T>
    void addComponent(Args&&... args)
    {
        addComponent(std::make_unique<T>(std::forward<Args>(args)...));
    }

    /// \brief Get a component from the object
    ///
    /// \tparam T type of the requested component. Must inherit from \ref Component
    ///
    /// \returns pointer to the component if it is present, nullptr otherwise
    template<typename T>
        requires std::is_base_of_v<Component, T>
    T* getComponent()
    {
        const auto& it{ m_components.find(typeid(T)) };
        if(it == m_components.cend())
            return nullptr;

        return static_cast<T*>(it->second.get());
    }

    /// \brief Get a component from the object
    ///
    /// \tparam T type of the requested component. Must inherit from \ref Component
    ///
    /// \returns pointer to const to the component if it is present, nullptr otherwise
    template<typename T>
        requires std::is_base_of_v<Component, T>
    const T* getComponent() const
    {
        const auto& it{ m_components.find(typeid(T)) };
        if(it == m_components.cend())
            return nullptr;

        return static_cast<const T*>(it->second.get());
    }

    static void resetIdPool() noexcept { s_idPool.reset(); }

private:
    inline static IdPool s_idPool{};

    ObjectId_t m_id{ INVALID_OBJECT_ID };
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_OBJECT_OBJECT_HPP
