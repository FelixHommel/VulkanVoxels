#include "Model.hpp"

#include <cstddef>
#include <vulkan/vulkan_core.h>

#include <array>
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

Model::Model(Device& device, const std::vector<Vertex>& vertices)
    : device(device)
    , m_vertexCount{ static_cast<std::uint32_t>(vertices.size()) }
{
    const VkDeviceSize bufferSize{ sizeof(vertices[0]) * m_vertexCount };

    device.createBuffer(bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_vertexBuffer,
            m_vertexBufferMemory);

    void* data{};
    vkMapMemory(device.device(), m_vertexBufferMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, vertices.data(), static_cast<std::size_t>(bufferSize));
    vkUnmapMemory(device.device(), m_vertexBufferMemory);
}

Model::~Model()
{
    vkDestroyBuffer(device.device(), m_vertexBuffer, nullptr);
    vkFreeMemory(device.device(), m_vertexBufferMemory, nullptr);
}

void Model::bind(const VkCommandBuffer commandBuffer) const
{
    const std::array<VkBuffer, 1> buffers{ m_vertexBuffer };
    constexpr std::array<VkDeviceSize, 1> offsets{ 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers.data(), offsets.data());
}

void Model::draw(const VkCommandBuffer commandBuffer) const
{
    vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
}


} // !vv
