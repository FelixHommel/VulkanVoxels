#include "Exception.hpp"

#include <source_location>
#include <sstream>
#include <string>
#include <utility>

namespace vv
{

Exception::Exception(std::string message, std::source_location location)
	: m_message{ std::move(message) }
	, m_location{ location }
{
	Exception::buildWhatMessage();
}
std::string Exception::detailedMessage() const
{
	// NOTE: On demand creation because there is no guarantee this will be called
	std::ostringstream oss;
	oss << "Exception: " << m_message << "\n"
		<< " at " << m_location.file_name() << ":" << m_location.line() << " in " << m_location.function_name() << "\n";

	return oss.str();
}

/// \brief Create the message that describes the reason for the exception
///
/// Takes the details and formats them into a presentable layout
void Exception::buildWhatMessage()
{
	std::ostringstream oss;
	oss << m_message << "[" << m_location.file_name() << ":" << m_location.line() << "]";

	m_message = oss.str();
}

} // namespace vv