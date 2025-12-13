#include "Model.hpp"

#include "core/Device.hpp"
#include "utility/Utils.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "external/tiny_obj_loader.h"
#include "glm/gtx/hash.hpp"
#include <vulkan/vulkan_core.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
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

} // namespace std

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
                v.position = { attrib.vertices[(3 * static_cast<std::size_t>(i.vertex_index)) + 0],
                               attrib.vertices[(3 * static_cast<std::size_t>(i.vertex_index)) + 1],
                               attrib.vertices[(3 * static_cast<std::size_t>(i.vertex_index)) + 2] };

                v.color = { attrib.colors[(3 * static_cast<std::size_t>(i.vertex_index)) + 0],
                            attrib.colors[(3 * static_cast<std::size_t>(i.vertex_index)) + 1],
                            attrib.colors[(3 * static_cast<std::size_t>(i.vertex_index)) + 2] };
            }

            if(i.normal_index >= 0)
            {
                v.normal = { attrib.normals[(3 * static_cast<std::size_t>(i.normal_index)) + 0],
                             attrib.normals[(3 * static_cast<std::size_t>(i.normal_index)) + 1],
                             attrib.normals[(3 * static_cast<std::size_t>(i.normal_index)) + 2] };
            }

            if(i.texcoord_index >= 0)
            {
                v.uv = {
                    attrib.texcoords[(2 * static_cast<std::size_t>(i.texcoord_index)) + 0],
                    attrib.texcoords[(2 * static_cast<std::size_t>(i.texcoord_index)) + 1],
                };
            }

            if(!uniqueVertices.contains(v))
            {
                uniqueVertices[v] = static_cast<std::uint32_t>(vertices.size());
                vertices.push_back(v);
            }
            indices.push_back(uniqueVertices[v]);
        }
    }
}

Model::Model(std::shared_ptr<Device> device, const Builder& builder) : device{ std::move(device) }
{
    createVertexBuffer(builder.vertices);
    createIndexBuffer(builder.indices);
}

std::unique_ptr<Model> Model::loadFromFile(std::shared_ptr<Device> device, const std::filesystem::path& filepath)
{
    Builder builder{};
    builder.loadModel(filepath);

    return std::make_unique<Model>(device, builder);
}

void Model::bind(VkCommandBuffer commandBuffer) const
{
    const std::array<VkBuffer, 1> buffers{ m_vertexBuffer->getBuffer() };
    constexpr std::array<VkDeviceSize, 1> offsets{ 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers.data(), offsets.data());

    if(m_hasIndexBuffer)
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void Model::draw(VkCommandBuffer commandBuffer) const
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
    constexpr std::uint32_t vertexSize{ sizeof(vertices[0]) };
    const VkDeviceSize bufferSize{ static_cast<VkDeviceSize>(vertexSize * m_vertexCount) };
#if defined(VV_ENABLE_ASSERTS)
    assert(m_vertexCount > 3 && "The Model must at least contain 3 vertices");
#endif

    Buffer stagingBuffer{ Buffer::createStagingBuffer(device, vertexSize, m_vertexCount) };
    stagingBuffer.writeToBuffer(vertices);
    stagingBuffer.flush();

    m_vertexBuffer = std::make_unique<Buffer>(Buffer::createVertexBuffer(device, vertexSize, m_vertexCount));

    device->copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
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

    constexpr std::uint32_t indexSize{ sizeof(indices[0]) };
    const VkDeviceSize bufferSize{ static_cast<VkDeviceSize>(indexSize * m_indexCount) };

    Buffer stagingBuffer{ Buffer::createStagingBuffer(device, indexSize, m_indexCount) };
    stagingBuffer.writeToBuffer(indices);
    stagingBuffer.flush();

    m_indexBuffer = std::make_unique<Buffer>(Buffer::createIndexBuffer(device, indexSize, m_indexCount));

    device->copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), bufferSize);
}


} // namespace vv
