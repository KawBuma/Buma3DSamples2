#pragma once

#include <Utils/private/LoggerDetails.h>

namespace buma
{

namespace log = spdlog;

//namespace log
//{
//
//enum class TYPE
//{
//      TRACE
//    , DEBUG
//    , INFO
//    , WARN
//    , ERROR
//    , CRITICAL
//};
//
//struct Logger
//{
//public:
//    void log(TYPE _type, const char* _msg);
//    void trace    (const char* _msg) { log(TYPE::TRACE   , _msg); }
//    void debug    (const char* _msg) { log(TYPE::DEBUG   , _msg); }
//    void info     (const char* _msg) { log(TYPE::INFO    , _msg); }
//    void warn     (const char* _msg) { log(TYPE::WARN    , _msg); }
//    void error    (const char* _msg) { log(TYPE::ERROR   , _msg); }
//    void critical (const char* _msg) { log(TYPE::CRITICAL, _msg); }
//
//public:
//
//};
//
//void SetFileOutputEnabled(bool _enabled = false, const char* _path = "./");
//
//void log(LOG_TYPE _type, const char* _msg);
//void trace    (const char* _msg) { log(LOG_TYPE_TRACE   , _msg); }
//void debug    (const char* _msg) { log(LOG_TYPE_DEBUG   , _msg); }
//void info     (const char* _msg) { log(LOG_TYPE_INFO    , _msg); }
//void warn     (const char* _msg) { log(LOG_TYPE_WARN    , _msg); }
//void error    (const char* _msg) { log(LOG_TYPE_ERROR   , _msg); }
//void critical (const char* _msg) { log(LOG_TYPE_CRITICAL, _msg); }
//
//
//} // namespace log
} // namespace buma


// 利便性のためのマクロ定義(各翻訳単位での使用を推奨)
//#define logt(...) buma::log::trace(__VA_ARGS__)
//#define logd(...) buma::log::debug(__VA_ARGS__)
//#define logi(...) buma::log::info(__VA_ARGS__)
//#define logw(...) buma::log::warn(__VA_ARGS__)
//#define loge(...) buma::log::error(__VA_ARGS__)
//#define logc(...) buma::log::critical(__VA_ARGS__)
