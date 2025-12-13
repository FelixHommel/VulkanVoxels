#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"

#include <memory>

int main(int argc, char** argv)
{
    // NOTE: Deactivate logging for tests
    auto nullSink{ std::make_shared<spdlog::sinks::null_sink_mt>() };
    auto nullLogger{ std::make_shared<spdlog::logger>("nullLogger", nullSink) };
    spdlog::set_default_logger(nullLogger);
    spdlog::set_level(spdlog::level::off);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
