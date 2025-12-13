#include "utility/exceptions/Exception.hpp"
#include "utility/exceptions/FileException.hpp"
#include "utility/exceptions/ResourceException.hpp"
#include "utility/exceptions/VulkanException.hpp"

#include "gtest/gtest.h"
#include <vulkan/vulkan_core.h>

#include <source_location>
#include <sstream>
#include <string>
#include <string_view>

namespace
{

std::string excptionMessageBuilder(std::string_view message, const std::source_location& location)
{
    std::ostringstream oss;
    oss << message << "[" << location.file_name() << ":" << location.line() << "]";

    return oss.str();
}

std::string excptionDetailedMessageBuilder(std::string_view message, const std::source_location& location)
{
    std::ostringstream oss;
    oss << "Exception: " << message << "\n"
        << " at " << location.file_name() << ":" << location.line() << " in " << location.function_name() << "\n";

    return oss.str();
}

} // namespace

namespace vv::test
{

TEST(ExceptionTest, MessageFormat)
{
    constexpr auto message{ "test" };
    const Exception e(message);

    const std::string expectedMessage{ ::excptionMessageBuilder(message, e.location()) };
    EXPECT_EQ(e.message(), message);
    EXPECT_EQ(e.what(), expectedMessage);
}

TEST(ExceptionTest, ExtendedMessageFormat)
{
    constexpr auto message{ "test" };
    const Exception e(message);

    EXPECT_EQ(e.detailedMessage(), ::excptionDetailedMessageBuilder(message, e.location()));
}

TEST(ExceptionTest, BaseClassCatchesChildClasses)
{
    EXPECT_THROW({ throw VulkanException("error", VK_ERROR_OUT_OF_DATE_KHR); }, Exception);
    EXPECT_THROW({ throw FileException("error", "path/"); }, Exception);
    EXPECT_THROW({ throw ResourceException("error"); }, Exception);
}

} // namespace vv::test
