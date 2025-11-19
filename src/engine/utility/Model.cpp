#include "Model.hpp"

#include <vulkan/vulkan_core.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <vector>

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
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
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
