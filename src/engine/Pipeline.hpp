#ifndef SRC_ENGINE_PIPELINE_HPP
#define SRC_ENGINE_PIPELINE_HPP

#include <filesystem>

namespace vv
{

/// \brief The Pipeline is a building piece that consits of programable and non-programable stages that vertices run through before they are displayed
///
/// \author Felix Hommel
/// \date 11/10/2025
class Pipeline
{
public:
    Pipeline(const std::filesystem::path& vertexShaderPath, const std::filesystem::path& fragmentShaderPath);
    ~Pipeline() = default;

    Pipeline(const Pipeline&) = default;
    Pipeline(Pipeline&&) = delete;
    Pipeline& operator=(const Pipeline&) = default;
    Pipeline& operator=(Pipeline&&) = delete;
};

}; // !vv

#endif // !SRC_ENGINE_PIPELINE_HPP
