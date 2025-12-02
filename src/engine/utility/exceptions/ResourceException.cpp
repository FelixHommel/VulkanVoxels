#include "ResourceException.hpp"

#include "utility/exceptions/Exception.hpp"

#include <source_location>
#include <string>
#include <utility>

namespace vv
{

ResourceException::ResourceException(
	std::string message,
	std::source_location location
)
	: Exception{ std::move(message), location }
{
}

} // namespace vv