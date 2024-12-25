//
// Created by JekYUlll on 2024/9/23.
//

#ifndef KF_LOG_LOGGER_H
#define KF_LOG_LOGGER_H

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include <cassert>
#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/common.h"

namespace kfc {

    class Logger {
    public:
        Logger() = delete;
        Logger(const Logger&) = delete;
        Logger &operator=(const Logger&) = delete;

        static void Init();

        static spdlog::logger* GetLoggerInstance() {
            assert(sLoggerInstance && "Logger instance is null, have not execute Logger::Init().");
            return sLoggerInstance.get();
        }

    private:
        static std::shared_ptr<spdlog::logger> sLoggerInstance;
    };

#define LOG_T(...) SPDLOG_LOGGER_TRACE(kfc::Logger::GetLoggerInstance(), __VA_ARGS__)
#define LOG_D(...) SPDLOG_LOGGER_DEBUG(kfc::Logger::GetLoggerInstance(), __VA_ARGS__)
#define LOG_I(...)  SPDLOG_LOGGER_INFO(kfc::Logger::GetLoggerInstance(), __VA_ARGS__)
#define LOG_W(...)  SPDLOG_LOGGER_WARN(kfc::Logger::GetLoggerInstance(), __VA_ARGS__)
#define LOG_E(...) SPDLOG_LOGGER_ERROR(kfc::Logger::GetLoggerInstance(), __VA_ARGS__)

}

#endif //KF_LOG_LOGGER_H
