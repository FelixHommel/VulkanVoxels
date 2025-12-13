#ifndef VULKAN_VOXELS_SRC_ENGINE_CORE_COMPUTE_PIPELINE_HPP
#define VULKAN_VOXELS_SRC_ENGINE_CORE_COMPUTE_PIPELINE_HPP

#include "core/Device.hpp"
#include "core/IPipeline.hpp"

#include <vulkan/vulkan_core.h>

#include <filesystem>
#include <memory>

namespace vv
{

/// \brief Pipelines to dispatch and run compute shaders and compute workloads
///
/// \author Felix Hommel
/// \date 11/10/2025
class ComputePipeline final : public IPipeline
{
public:
    /// \brief Create a new \ref ComputePipeline
    ///
    /// \param device \ref Device where the pipeline is created on
    /// \param computeShaderPath path to the compute shader as compiled spir-v binary
    /// \param pipelineLayout the layout of the pipeline's descriptors
    ComputePipeline(
        std::shared_ptr<Device> device,
        const std::filesystem::path& computeShaderPath,
        const VkPipelineLayout& pipelineLayout
    );
    ~ComputePipeline() override;

    ComputePipeline(const ComputePipeline&) = delete;
    ComputePipeline(ComputePipeline&&) = delete;
    ComputePipeline& operator=(const ComputePipeline&) = delete;
    ComputePipeline& operator=(ComputePipeline&&) = delete;

    /// \brief Bind the Pipeline to a command buffer
    ///
    /// \param commandBuffer the VkCommandBuffer to which the pipeline is being bound
    void bind(VkCommandBuffer commandBuffer) const override;

private:
    VkShaderModule m_computeShaderModule{};
};

}; // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_CORE_COMPUTE_PIPELINE_HPP
