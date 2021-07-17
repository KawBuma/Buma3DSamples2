#pragma once

#pragma region compile-time log output filtering

#define BUMA_LOGTYPE_ALL      0
#define BUMA_LOGTYPE_TRACE    0
#define BUMA_LOGTYPE_DEBUG    1
#define BUMA_LOGTYPE_INFO     2
#define BUMA_LOGTYPE_WARN     3
#define BUMA_LOGTYPE_ERROR    4
#define BUMA_LOGTYPE_CRITICAL 5

#ifndef BUMA_MIN_LOGTYPE
#define BUMA_MIN_LOGTYPE BUMA_LOGTYPE_INFO
#endif // !BUMA_MIN_LOGTYPE

#if BUMA_MIN_LOGTYPE <= BUMA_LOGTYPE_TRACE
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#elif BUMA_MIN_LOGTYPE <= BUMA_LOGTYPE_DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#elif BUMA_MIN_LOGTYPE <= BUMA_LOGTYPE_INFO
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#elif BUMA_MIN_LOGTYPE <= BUMA_LOGTYPE_WARN
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_WARN

#elif BUMA_MIN_LOGTYPE <= BUMA_LOGTYPE_ERROR
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_ERROR

#elif BUMA_MIN_LOGTYPE <= BUMA_LOGTYPE_CRITICAL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_CRITICAL
#endif

#define BUMA_LOGT(...) SPDLOG_TRACE(__VA_ARGS__)
#define BUMA_LOGD(...) SPDLOG_DEBUG(__VA_ARGS__)
#define BUMA_LOGI(...) SPDLOG_INFO(__VA_ARGS__)
#define BUMA_LOGW(...) SPDLOG_WARN(__VA_ARGS__)
#define BUMA_LOGE(...) SPDLOG_ERROR(__VA_ARGS__)
#define BUMA_LOGC(...) SPDLOG_CRITICAL(__VA_ARGS__)

#define BUMA_LOGGERT(logger, ...) SPDLOG_LOGGER_TRACE(logger, __VA_ARGS__)
#define BUMA_LOGGERD(logger, ...) SPDLOG_LOGGER_DEBUG(logger, __VA_ARGS__)
#define BUMA_LOGGERI(logger, ...) SPDLOG_LOGGER_INFO(logger, __VA_ARGS__)
#define BUMA_LOGGERW(logger, ...) SPDLOG_LOGGER_WARN(logger, __VA_ARGS__)
#define BUMA_LOGGERE(logger, ...) SPDLOG_LOGGER_ERROR(logger, __VA_ARGS__)
#define BUMA_LOGGERC(logger, ...) SPDLOG_LOGGER_CRITICAL(logger, __VA_ARGS__)

#pragma endregion compile-time log output filtering

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
