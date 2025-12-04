#include "VulkanException.hpp"

#include "utility/exceptions/Exception.hpp"

#include <vulkan/vulkan_core.h>

#include <source_location>
#include <sstream>
#include <string>
#include <utility>

namespace vv
{

VulkanException::VulkanException(std::string message, VkResult result, std::source_location location)
    : Exception{ std::move(message), location }
    , m_result{ result }
{
    VulkanException::buildWhatMessage();
}

std::string VulkanException::detailedMessage() const
{
    std::ostringstream oss;
    oss << "Vulkan Error: " << m_message << "\n"
        << " Result code: " << vkResultToString(m_result) << "(" << m_result << ")\n"
        << " at " << m_location.file_name() << ":" << m_location.line() << " in " << m_location.function_name() << "\n"
        << "Stack trace:\n";

    return oss.str();
}

void VulkanException::buildWhatMessage()
{
    std::ostringstream oss;
    oss << m_message << " (VkResult: " << vkResultToString(m_result) << ")"
        << " [" << m_location.file_name() << ":" << m_location.line() << "]";

    m_whatMessage = oss.str();
}

/// \brief Convert a VkResult to string
///
/// \param result the VkResult type
///
/// \returns string version of \p result
const char* VulkanException::vkResultToString(VkResult result)
{
    switch(result)
    {
    case VK_SUCCESS:
        return "VK_SUCCESS";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
        return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_SUBOPTIMAL_KHR:
        return "VK_SUBOPTIMAL_KHR";
    default:
        return "UNKNOWN_VK_ERROR";
    }
}

} // namespace vv