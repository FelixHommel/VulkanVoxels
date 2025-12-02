#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_EXCEPTIONS_RESOURCE_EXCEPTION_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_EXCEPTIONS_RESOURCE_EXCEPTION_HPP

#include "utility/exceptions/Exception.hpp"

#include <source_location>
#include <string>

namespace vv
{

/// \brief Exception type for errors with general resources like buffers, objects, ...
///
/// \author Felix Hommel
/// \date 12/1/2025
class ResourceException : public Exception
{
public:
	/// \brief Create a new \ref VulkanException
	///
	/// \param message the exception message
	/// \param location (optional) where the exception is thrown
	explicit ResourceException(
		std::string message,
		std::source_location location = std::source_location::current()
	);

private:
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_EXCEPTIONS_RESOURCE_EXCEPTION_HPP