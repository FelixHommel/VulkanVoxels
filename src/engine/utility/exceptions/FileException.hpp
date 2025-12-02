#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_EXCEPTIONS_FILE_EXCEPTION_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_EXCEPTIONS_FILE_EXCEPTION_HPP

#include "utility/exceptions/Exception.hpp"

#include <source_location>
#include <string>

namespace vv
{

/// \brief Exception type for File IO errors
///
/// \author Felix Hommel
/// \date 11/30/2025
class FileException final : public Exception
{
public:
	/// \brief Create a new \ref VulkanException
	///
	/// \param message the exception message
	/// \param filepath the path to the file that caused the exception
	/// \param location (optional) where the exception is thrown
	explicit FileException(
		std::string message,
		std::string filepath,
		std::source_location location = std::source_location::current()
	);

	[[nodiscard]] const std::string& filepath() const noexcept { return m_filepath; }

	[[nodiscard]] std::string detailedMessage() const override;

private:
	std::string m_filepath;

	void buildWhatMessage() override;
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_EXCEPTIONS_FILE_EXCEPTION_HPP