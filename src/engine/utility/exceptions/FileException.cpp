#include "FileException.hpp"

#include "utility/exceptions/Exception.hpp"

#include <source_location>
#include <sstream>
#include <string>
#include <utility>

namespace vv
{

FileException::FileException(
	std::string message,
	std::string filepath,
	std::source_location location
)
	: Exception{ std::move(message), location }
	, m_filepath{ std::move(filepath) }
{
	FileException::buildWhatMessage();
}

std::string FileException::detailedMessage() const
{
	std::ostringstream oss;
	oss << "File IO Error(" << m_filepath << "): " << m_message << "\n"
		<< " at " << m_location.file_name() << ":" << m_location.line()
		<< " in " << m_location.function_name() << "\n"
		<< "Stack trace:\n";

	return oss.str();
}

void FileException::buildWhatMessage()
{
	std::ostringstream oss;
	oss << m_message << " (file: " << m_filepath << ")"
		<< " [" << m_location.file_name() << ":" << m_location.line() << "]";

	m_whatMessage = oss.str();
}

} // namespace  vv