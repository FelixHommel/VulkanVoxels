#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_MODEL_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_MODEL_HPP

#include "core/Buffer.hpp"
#include "core/Device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <filesystem>
#include <memory>
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
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 uv{};

        /// \brief Provide the information about the Binding that the Pipeline needs
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        /// \brief Provide the information about the Attributes that the Pipeline needs
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        bool operator==(const Vertex& other) const
        {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
        }
    };

    /// \brief Helper struct that stores the vertices and potential indices of a mesh
    ///
    /// \author Felix Hommel
    /// \date 11/19/2025
    struct Builder
    {
        std::vector<Vertex> vertices;
        std::vector<std::uint32_t> indices;

        /// \brief Use tinyobj to parse obj file and carve out vertex and index buffer content
        ///
        /// \param filepath path to the obj file
        void loadModel(const std::filesystem::path& filepath);
    };

    /// \brief Create a new \ref Model
    ///
    /// \param device \ref Device that is used to allocate the vertex buffer memory
    /// \param vertices vector containing the vertices that make up the Model Mesh
    Model(Device& device, const Builder& builder);
    ~Model() = default;

    Model(const Model&) = delete;
    Model(Model&&) = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = delete;

    /// \brief Load a .obj file
    ///
    /// \param device \ref Device to create the vertex and index buffers on
    /// \param fileapth path to the obj file
    static std::unique_ptr<Model> loadFromFile(Device& device, const std::filesystem::path& filepath);

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

    std::unique_ptr<Buffer> m_vertexBuffer;
    std::uint32_t m_vertexCount{};

    bool m_hasIndexBuffer{ false };
    std::unique_ptr<Buffer> m_indexBuffer;
    std::uint32_t m_indexCount{};

    void createVertexBuffer(const std::vector<Vertex>& vertices);
    void createIndexBuffer(const std::vector<std::uint32_t>& indices);
};

} // !vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_MODEL_HPP
