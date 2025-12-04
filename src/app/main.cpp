#include "Application.hpp"
#include "utility/exceptions/Exception.hpp"
#include "utility/exceptions/FileException.hpp"
#include "utility/exceptions/ResourceException.hpp"
#include "utility/exceptions/VulkanException.hpp"

#include "spdlog/common.h"
#include "spdlog/spdlog.h"

#include <exception>

int main()
{
    try
    {
        vv::Application app{};
        app.run();
    }
    catch(const vv::VulkanException& e)
    {
        spdlog::critical("Vulkan error: {}", e.detailedMessage());
        return 1;
    }
    catch(const vv::FileException& e)
    {
        spdlog::critical("File error: {}", e.detailedMessage());
        return 1;
    }
    catch(const vv::ResourceException& e)
    {
        spdlog::critical("Resource error: {}", e.detailedMessage());
        return 1;
    }
    catch(const vv::Exception& e)
    {
        spdlog::critical("Application error: {}", e.detailedMessage());
        return 1;
    }
    catch(const std::exception& e)
    {
        spdlog::critical("Unexpected error: {}", e.what());
        return 1;
    }

    return 0;
}
