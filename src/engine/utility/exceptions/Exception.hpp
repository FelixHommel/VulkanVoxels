#ifndef VULKAN_VOXELS_SRC_ENGINE_UTILITY_EXCEPTIONS_EXCEPTION_HPP
#define VULKAN_VOXELS_SRC_ENGINE_UTILITY_EXCEPTIONS_EXCEPTION_HPP

#include <exception>
#include <source_location>
#include <string>
#include <string_view>

namespace vv
{

/// \brief Base class exception for unclassified exceptions
///
/// \author Felix Hommel
/// \date 11/30/2025
class Exception : public std::exception
{
public:
	/// \brief Create a new \ref Exception
	///
	/// \param message the exception message
	/// \param location (optional) where the exception is thrown
	explicit Exception(std::string message, std::source_location location = std::source_location::current());

	/// \brief Why the exception was thrown
	///
	/// \returns const char* with the reason why this exception was thrown
	[[nodiscard]] const char* what() const noexcept override { return m_message.c_str(); }
	/// \brief Why the exception was thrown in short form
	///
	/// \returns std::string_view with the reason why the exception was thrown
	[[nodiscard]] std::string_view message() const noexcept { return m_message; }
	[[nodiscard]] const std::source_location& location() const noexcept { return m_location; }
	/// \brief An extended version of the message() function that provides more details
	///
	/// \returns std::string with a more detailed message
	[[nodiscard]] virtual std::string detailedMessage() const;

protected:
	std::string m_message;
	std::source_location m_location;
	std::string m_whatMessage;

	virtual void buildWhatMessage();
};

} // namespace vv

#endif // !VULKAN_VOXELS_SRC_ENGINE_UTILITY_EXCEPTIONS_EXCEPTION_HPP
