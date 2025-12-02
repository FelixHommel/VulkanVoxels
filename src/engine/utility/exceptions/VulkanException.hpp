#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_EXCEPTIONS_VULKAN_EXCEPTION_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_EXCEPTIONS_VULKAN_EXCEPTION_HPP

#include "utility/exceptions/Exception.hpp"

#include <vulkan/vulkan_core.h>

#include <source_location>
#include <string>

namespace vv
{

/// \brief Exception type for vulkan errors
///
/// \author Felix Hommel
/// \date 11/30/2925
class VulkanException final : public Exception
{
public:
	/// \brief Create a new \ref VulkanException
	///
	/// \param message the exception message
	/// \param result the VkResult that lead to the exception
	/// \param location (optional) where the exception is thrown
	explicit VulkanException(
		std::string message,
		VkResult result,
		std::source_location location = std::source_location::current()
	);

	[[nodiscard]] VkResult result() const noexcept { return m_result; }

	[[nodiscard]] std::string detailedMessage() const override;

private:
	VkResult m_result;

	void buildWhatMessage() override;
	static const char* vkResultToString(VkResult result);
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_EXCEPTIONS_VULKAN_EXCEPTION_HPP
