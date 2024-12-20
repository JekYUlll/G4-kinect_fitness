//
// Created by JekYUlll on 2024/9/23.
//
#include "log/KFLog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/async.h"

namespace kf {

    std::shared_ptr<spdlog::logger> Logger::sLoggerInstance{};

    void Logger::Init() {
        sLoggerInstance = spdlog::stdout_color_mt<spdlog::async_factory>("async_logger");
        sLoggerInstance->set_level(spdlog::level::trace);
        sLoggerInstance->set_pattern("%^%H:%M:%S:%e [%P-%t] [%1!L] [%20s:%-4#] - %v%$");
    }

}