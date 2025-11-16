#ifndef SRC_ENGINE_BASIC_RENDER_SYSTEM_HPP
#define SRC_ENGINE_BASIC_RENDER_SYSTEM_HPP

#include "Object.hpp"
#include "Device.hpp"
#include "Pipeline.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include <vulkan/vulkan_core.h>

#include <memory>

namespace vv
{

struct SimplePushConstantData
{
    glm::mat2 transform{ 1.f };
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

class BasicRenderSystem
{
public:
    BasicRenderSystem(Device& device, VkRenderPass renderPass);
    ~BasicRenderSystem();

    BasicRenderSystem(const BasicRenderSystem&) = delete;
    BasicRenderSystem(BasicRenderSystem&&) = delete;
    BasicRenderSystem& operator=(const BasicRenderSystem&) = delete;
    BasicRenderSystem& operator=(BasicRenderSystem&&) = delete;

    void renderObjects(VkCommandBuffer commandBuffer, std::vector<Object>& objects);

private:
    static constexpr auto VERTEX_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/simpleVert.spv" };
    static constexpr auto FRAGMENT_SHADER_PATH{ PROJECT_ROOT "resources/compiledShaders/simpleFrag.spv" };

    Device& device;

    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };

    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);
};

} // !vv

#endif // !SRC_ENGINE_BASIC_RENDER_SYSTEM_HPP
