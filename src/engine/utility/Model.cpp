#include "Model.hpp"

#include "core/Device.hpp"
#include "utility/Utils.hpp"
#include <memory>
#include <stdexcept>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"
#include "external/tiny_obj_loader.h"
#include <vulkan/vulkan_core.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <vector>

namespace std
{

template<>
struct hash<vv::Model::Vertex>
{
    std::size_t operator()(const vv::Model::Vertex& vertex) const
    {
        std::size_t seed{ 0 };
        vv::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);

        return seed;
    }
};

} // !std

namespace vv
{

std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions()
{
    // NOTE: Per member variable of the Vertex a entry in the attribute description is needed
    // Order: location, binding, format, offset
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    attributeDescriptions.emplace_back(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
    attributeDescriptions.emplace_back(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
    attributeDescriptions.emplace_back(2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal));
    attributeDescriptions.emplace_back(3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv));

    return attributeDescriptions;
}

void Model::Builder::loadModel(const std::filesystem::path& filepath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning;
    std::string error;

    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, filepath.c_str()))
        throw std::runtime_error(warning + error);

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, std::uint32_t> uniqueVertices{};
    for(const auto& s : shapes)
    {
        for(const auto& i : s.mesh.indices)
        {
            Vertex v{};

            if(i.vertex_index >= 0)
            {
                v.position = {
                    attrib.vertices[static_cast<std::size_t>(3 * i.vertex_index + 0)],
                    attrib.vertices[static_cast<std::size_t>(3 * i.vertex_index + 1)],
                    attrib.vertices[static_cast<std::size_t>(3 * i.vertex_index + 2)]
                };

                v.color = {
                    attrib.colors[static_cast<std::size_t>(3 * i.vertex_index + 0)],
                    attrib.colors[static_cast<std::size_t>(3 * i.vertex_index + 1)],
                    attrib.colors[static_cast<std::size_t>(3 * i.vertex_index + 2)]
                };
            }

            if(i.normal_index >= 0)
            {
                v.normal = {
                    attrib.normals[static_cast<std::size_t>(3 * i.normal_index + 0)],
                    attrib.normals[static_cast<std::size_t>(3 * i.normal_index + 1)],
                    attrib.normals[static_cast<std::size_t>(3 * i.normal_index + 2)]
                };
            }

            if(i.texcoord_index >= 0)
            {
                v.uv = {
                    attrib.texcoords[static_cast<std::size_t>(2 * i.texcoord_index + 0)],
                    attrib.texcoords[static_cast<std::size_t>(2 * i.texcoord_index + 1)],
                };
            }

            if(uniqueVertices.count(v) == 0)
            {
                uniqueVertices[v] = static_cast<std::uint32_t>(vertices.size());
                vertices.push_back(v);
            }
            indices.push_back(uniqueVertices[v]);
        }
    }
}

Model::Model(Device& device, const Builder& builder)
    : device(device)
{
    createVertexBuffer(builder.vertices);
    createIndexBuffer(builder.indices);
}

Model::~Model()
{
    vkDestroyBuffer(device.device(), m_vertexBuffer, nullptr);
    vkFreeMemory(device.device(), m_vertexBufferMemory, nullptr);

    if(m_hasIndexBuffer)
    {
        vkDestroyBuffer(device.device(), m_indexBuffer, nullptr);
        vkFreeMemory(device.device(), m_indexBufferMemory, nullptr);
    }
}

std::unique_ptr<Model> Model::loadFromFile(Device& device, const std::filesystem::path& filepath)
{
    Builder builder{};
    builder.loadModel(filepath);

    return std::make_unique<Model>(device, builder);
}

void Model::bind(const VkCommandBuffer commandBuffer) const
{
    const std::array<VkBuffer, 1> buffers{ m_vertexBuffer };
    constexpr std::array<VkDeviceSize, 1> offsets{ 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers.data(), offsets.data());

    if(m_hasIndexBuffer)
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void Model::draw(const VkCommandBuffer commandBuffer) const
{
    if(m_hasIndexBuffer)
        vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
    else
        vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
}

/// \brief Create a new Vertex Buffer using the data specified by the vertices
///
/// Uses a staging buffer to transfer the vertices to device local memory.
///
/// \param vertices vertex data that is stored in the buffer
void Model::createVertexBuffer(const std::vector<Vertex>& vertices)
{
    m_vertexCount = static_cast<std::uint32_t>(vertices.size());
    const VkDeviceSize bufferSize{ sizeof(vertices[0]) * m_vertexCount };

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingMemory);

    void* data;
    vkMapMemory(device.device(), stagingMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, vertices.data(), static_cast<std::size_t>(bufferSize));
    vkUnmapMemory(device.device(), stagingMemory);

    device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_vertexBuffer,
            m_vertexBufferMemory);

    device.copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
    vkFreeMemory(device.device(), stagingMemory, nullptr);
}

/// \brief Create a new Index Buffer using the data specified by the indices
///
/// Uses a staging buffer to transfer the indices to device local memory.
///
/// \param indices index data that is stored in the buffer
void Model::createIndexBuffer(const std::vector<std::uint32_t>& indices)
{
    m_indexCount = static_cast<std::uint32_t>(indices.size());
    m_hasIndexBuffer = m_indexCount > 0;

    if(!m_hasIndexBuffer)
        return;

    const VkDeviceSize bufferSize{ sizeof(indices[0]) * m_indexCount };

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingMemory);

    void* data;
    vkMapMemory(device.device(), stagingMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, indices.data(), static_cast<std::size_t>(bufferSize));
    vkUnmapMemory(device.device(), stagingMemory);

    device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_indexBuffer,
            m_indexBufferMemory);

    device.copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
    vkFreeMemory(device.device(), stagingMemory, nullptr);
}


} // !vv
