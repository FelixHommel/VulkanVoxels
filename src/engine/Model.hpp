#ifndef SRC_ENGINE_MODEL_HPP
#define SRC_ENGINE_MODEL_HPP

#include "Device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <vector>

namespace vv
{

/// \brief The Model is an abstraction of Meshes that can represent various things
///
/// Create a vertex buffer and manage the memory needed to save the vertices
///
/// \author Felix Hommel
/// \date 11/19/2025
class Model
{
public:
    /// \brief Information that every vertex contains
    ///
    /// \author Felix Hommel
    /// \date 11/19/2025
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;

        /// \brief Provide the information about the Binding that the Pipeline needs
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        /// \brief Provide the information about the Attributes that the Pipeline needs
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    /// \brief Create a new \ref Model
    ///
    /// \param device \ref Device that is used to allocate the vertex buffer memory
    /// \param vertices vector containing the vertices that make up the Model Mesh
    Model(Device& device, const std::vector<Vertex>& vertices);
    ~Model();

    Model(const Model&) = delete;
    Model(Model&&) = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = delete;

    /// \brief Bind the vertex buffer of the model
    ///
    /// \param commandBuffer the VkCommandBuffer that the vertex buffer is bound to
    void bind(VkCommandBuffer commandBuffer) const;
    /// \brief Draw the vertices in the vertex buffer
    ///
    /// \param commandBuffer the VkCommandBuffer that the vertices are drawn to
    void draw(VkCommandBuffer commandBuffer) const;

private:
    Device& device;
    VkBuffer m_vertexBuffer{ VK_NULL_HANDLE };
    VkDeviceMemory m_vertexBufferMemory{ VK_NULL_HANDLE };
    std::uint32_t m_vertexCount;
};

} // !vv

#endif // !SRC_ENGINE_MODEL_HPP
