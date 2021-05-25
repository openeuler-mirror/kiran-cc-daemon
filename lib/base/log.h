/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 16:03:46
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 15:49:04
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/lib/base/log.h
 */
#pragma once

#include <glibmm.h>

#include "lib/base/def.h"

namespace Kiran
{
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

class Log
{
public:
    Log();
    Log(const Log &) = delete;
    virtual ~Log(){};

    static Log *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit();

    void try_append(GLogLevelFlags log_level,
                    const std::string &file_name,
                    const std::string &function_name,
                    int32_t line_number,
                    const char *format, ...);

private:
    void init();

    static void log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);

private:
    static Log *instance_;

    static const int kMessageSize = 10240;
    char message_[kMessageSize];
};

#define LOG_FATAL(format, ...)                                   \
    do                                                           \
    {                                                            \
        Kiran::Log::get_instance()->try_append(G_LOG_FLAG_FATAL, \
                                               __FILENAME__,     \
                                               __FUNCTION__,     \
                                               __LINE__,         \
                                               format,           \
                                               ##__VA_ARGS__);   \
    } while (0)

#define LOG_ERROR(format, ...)                                    \
    do                                                            \
    {                                                             \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_ERROR, \
                                               __FILENAME__,      \
                                               __FUNCTION__,      \
                                               __LINE__,          \
                                               format,            \
                                               ##__VA_ARGS__);    \
    } while (0)

#define LOG_WARNING(format, ...)                                    \
    do                                                              \
    {                                                               \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_WARNING, \
                                               __FILENAME__,        \
                                               __FUNCTION__,        \
                                               __LINE__,            \
                                               format,              \
                                               ##__VA_ARGS__);      \
    } while (0)

#define LOG_INFO(format, ...)                                    \
    do                                                           \
    {                                                            \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_INFO, \
                                               __FILENAME__,     \
                                               __FUNCTION__,     \
                                               __LINE__,         \
                                               format,           \
                                               ##__VA_ARGS__);   \
    } while (0)

#define LOG_DEBUG(format, ...)                                    \
    do                                                            \
    {                                                             \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_DEBUG, \
                                               __FILENAME__,      \
                                               __FUNCTION__,      \
                                               __LINE__,          \
                                               format,            \
                                               ##__VA_ARGS__);    \
    } while (0)

#define SETTINGS_PROFILE_START(format, ...)                       \
    do                                                            \
    {                                                             \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_DEBUG, \
                                               __FILENAME__,      \
                                               __FUNCTION__,      \
                                               __LINE__,          \
                                               "START " format,   \
                                               ##__VA_ARGS__);    \
    } while (0)

#define SETTINGS_PROFILE_END(format, ...)                         \
    do                                                            \
    {                                                             \
        Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_DEBUG, \
                                               __FILENAME__,      \
                                               __FUNCTION__,      \
                                               __LINE__,          \
                                               "END " format,     \
                                               ##__VA_ARGS__);    \
    } while (0)

#define SETTINGS_PROFILE(format, ...)                                      \
    Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_DEBUG,              \
                                           __FILENAME__,                   \
                                           __FUNCTION__,                   \
                                           __LINE__,                       \
                                           "START " format,                \
                                           ##__VA_ARGS__);                 \
    SCOPE_EXIT({ Kiran::Log::get_instance()->try_append(G_LOG_LEVEL_DEBUG, \
                                                        __FILENAME__,      \
                                                        _arg_function,     \
                                                        __LINE__,          \
                                                        "END " format,     \
                                                        ##__VA_ARGS__); });

}  // namespace Kiran