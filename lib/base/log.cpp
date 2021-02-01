/**
 * @file          /kiran-cc-daemon/lib/base/log.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/base/log.h"

#include <zlog_ex.h>

#include <sstream>

namespace Kiran
{
Log::Log() : log_level_(G_LOG_LEVEL_WARNING),
             logger_(nullptr)
{
}

Log *Log::instance_ = nullptr;
void Log::global_init()
{
    RETURN_IF_TRUE(instance_);
    instance_ = new Log();
    instance_->init();
}

void Log::global_deinit()
{
    delete instance_;
    instance_ = nullptr;
}

void Log::try_append(GLogLevelFlags log_level,
                     const std::string &file_name,
                     const std::string &function_name,
                     int32_t line_number,
                     const char *format, ...)
{
    if (log_level > this->log_level_)
    {
        return;
    }

    int32_t priority;
    std::ostringstream oss;

    switch (log_level & G_LOG_LEVEL_MASK)
    {
    case G_LOG_FLAG_FATAL:
        priority = ZLOG_LEVEL_FATAL;
        oss << "[FATAL]";
        break;
    case G_LOG_LEVEL_ERROR:
        priority = ZLOG_LEVEL_ERROR;
        oss << "[ERROR]";
        break;
    case G_LOG_LEVEL_CRITICAL:
        priority = ZLOG_LEVEL_ERROR;
        oss << "[ERROR]";
        break;
    case G_LOG_LEVEL_WARNING:
        priority = ZLOG_LEVEL_WARN;
        oss << "[WARNING]";
        break;
    case G_LOG_LEVEL_MESSAGE:
        priority = ZLOG_LEVEL_NOTICE;
        oss << "[NOTICE]";
        break;
    case G_LOG_LEVEL_INFO:
        priority = ZLOG_LEVEL_INFO;
        oss << "[INFO]";
        break;
    case G_LOG_LEVEL_DEBUG:
        priority = ZLOG_LEVEL_DEBUG;
        oss << "[DEBUG]";
        break;
    default:
        priority = ZLOG_LEVEL_DEBUG;
        oss << "[UNKNOWN]";
        break;
    }

    va_list arg_ptr;
    va_start(arg_ptr, format);
    vsnprintf(this->message_, Log::kMessageSize, format, arg_ptr);
    va_end(arg_ptr);

    dzlog(file_name.c_str(),
          file_name.length(),
          function_name.c_str(),
          function_name.length(),
          line_number,
          priority,
          "%s",
          this->message_);

    if (this->logger_)
    {
        oss << " [" << file_name << ":" << line_number << "-" << function_name << "()] " << this->message_;
        auto log_content = oss.str();
        this->logger_->write_log(log_content.c_str(), log_content.length());
    }
}

void Log::init()
{
    g_log_set_default_handler(log_handler, this);
}

void Log::log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
    Log *log = (Log *)user_data;
    if (!log)
    {
        return;
    }

    log->try_append(log_level, "gtk-file", "gtk-function", 0, message);
}

}  // namespace Kiran