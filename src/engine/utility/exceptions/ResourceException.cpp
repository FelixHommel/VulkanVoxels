#include "ResourceException.hpp"

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