#include "Application.hpp"

#include "spdlog/common.h"
#include "spdlog/spdlog.h"

int main()
{
    vv::Application app{};

    try
    {
        app.run();
    }
    catch(const std::exception& e)
    {
        spdlog::log(spdlog::level::critical, "Error occured: {}", e.what());
        return 1;
    }

    return 0;
}
