#include "Pipeline.hpp"

#include "utility/FileIO.hpp"

#include "spdlog/common.h"
#include "spdlog/spdlog.h"
#include <stdexcept>
#include <vector>

namespace vv
{

Pipeline::Pipeline(const std::filesystem::path& vertexShaderPath, const std::filesystem::path& fragmentShaderPath)
{
    std::vector<char> vertexShaderCode{};
    std::vector<char> fragmentShaderCode{};
    try
    {
        vertexShaderCode = File::read(vertexShaderPath);
        fragmentShaderCode = File::read(fragmentShaderPath);
    }
    catch(const std::runtime_error& e)
    {
        throw;
    }

    spdlog::log(spdlog::level::info, "vertex shader size: {} | fragment shader size: {}", vertexShaderCode.size(), fragmentShaderCode.size());
}

} // !vv
