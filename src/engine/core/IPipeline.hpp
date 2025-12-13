#ifndef VULKAN_VOXELS_SRC_ENGINE_CORE_IPIPELINE_HPP
#define VULKAN_VOXELS_SRC_ENGINE_CORE_IPIPELINE_HPP

#include "core/Device.hpp"
#include "utility/exceptions/FileException.hpp"
#include "utility/exceptions/VulkanException.hpp"

#include <vulkan/vulkan_core.h>

#include <filesystem>
#include <fstream>
#include <memory>

namespace vv
{

/// \brief Interface for pipelines such as Compute or Graphics
///
/// \author Felix Hommel
/// \date 12/13/12025
class IPipeline
{
public:
    explicit IPipeline(std::shared_ptr<Device> device) : m_device(std::move(device)) {}
    virtual ~IPipeline() = default;

    IPipeline(const IPipeline&) = default;
    IPipeline(IPipeline&&) = delete;
    IPipeline& operator=(const IPipeline&) = default;
    IPipeline& operator=(IPipeline&&) = delete;

    /// \brief Bind the Pipeline to a command buffer
    ///
    /// \param commandBuffer the VkCommandBuffer to which the pipeline is being bound
    virtual void bind(VkCommandBuffer commandBuffer) const = 0;

protected:
    std::shared_ptr<Device> m_device;
    VkPipeline m_pipeline{ VK_NULL_HANDLE };

    /// \brief Create a new Shader
    ///
    /// \param code the compiled shader byte code
    /// \param pShaderModule where the shader module is saved
    void createShaderModule(const std::vector<char>& code, VkShaderModule* pShaderModule) const
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const std::uint32_t*>(code.data());

        const VkResult result{ vkCreateShaderModule(m_device->device(), &createInfo, nullptr, pShaderModule) };
        if(result != VK_SUCCESS)
            throw VulkanException("Failed to create shader", result);
    }

    /// \brief Read the file and return its content
    ///
    /// \param filepath std::filesystem::path to the location of the file
    ///
    /// \return std::vector of chars containing the file contents
    static std::vector<char> readFile(const std::filesystem::path& filepath)
    {
        std::ifstream fileHandle{};
        fileHandle.open(filepath, std::ios::ate | std::ios::binary);

        if(!fileHandle.is_open())
            throw FileException("Failed to open shader file", filepath);

        std::vector<char> buffer(fileHandle.tellg());

        fileHandle.seekg(0);
        fileHandle.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

        fileHandle.close();
        return buffer;
    }
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_CORE_IPIPELINE_HPP
