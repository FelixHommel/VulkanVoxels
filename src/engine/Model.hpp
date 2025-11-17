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

class Model
{
public:
    struct Vertex
    {
        glm::vec2 position;

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    Model(Device& device, const std::vector<Vertex>& vertices);
    ~Model();

    Model(const Model&) = delete;
    Model(Model&&) = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = delete;

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer) const;

private:
    Device& device;
    VkBuffer m_vertexBuffer{ VK_NULL_HANDLE };
    VkDeviceMemory m_vertexBufferMemory{ VK_NULL_HANDLE };
    std::uint32_t m_vertexCount;
};

} // !vv

#endif // !SRC_ENGINE_MODEL_HPP
