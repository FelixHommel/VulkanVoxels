#ifndef SRC_ENGINE_UTILITY_FILE_IO_HPP
#define SRC_ENGINE_UTILITY_FILE_IO_HPP

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace vv::File
{

/// \brief Read the file and return its content
///
/// \param filepath std::filesystem::path to the location of the file
///
/// \return std::vector of chars containing the file contents
static std::vector<char> read(const std::filesystem::path& filepath)
{
    std::ifstream fileHandle{};
    fileHandle.open(filepath, std::ios::ate | std::ios::binary);

    if(!fileHandle.is_open())
        throw std::runtime_error("Failed to open file: " + filepath.string());

    std::streamsize fileSize{ static_cast<std::streamsize>(fileHandle.tellg()) };
    std::vector<char> buffer(fileSize);

    fileHandle.seekg(0);
    fileHandle.read(buffer.data(), fileSize);

    fileHandle.close();
    return buffer;
}

} // !vv::File

#endif // !SRC_ENGINE_UTILITY_FILE_IO_HPP
