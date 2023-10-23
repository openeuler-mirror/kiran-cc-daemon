/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <fmt/format.h>

#include <cstdio>
#include <functional>
#include <vector>

namespace Kiran
{
#define GETTEXT_PACKAGE "kiran-cc-daemon"

#define SYSTEMD_NAME "org.freedesktop.systemd1"
#define SYSTEMD_PATH "/org/freedesktop/systemd1"
#define SYSTEMD_MANAGER_INTERFACE "org.freedesktop.systemd1.Manager"
#define SYSTEMD_UNIT_INTERFACE "org.freedesktop.systemd1.Unit"

#define POLKIT_NAME "org.freedesktop.PolicyKit1"
#define POLKIT_PATH "/org/freedesktop/PolicyKit1/Authority"
#define POLKIT_INTERFACE "org.freedesktop.PolicyKit1.Authority"

#define EPS 1e-4

#define CHECK_XMLPP_ELEMENT(node, err)                                                                       \
    {                                                                                                        \
        const auto element = dynamic_cast<const xmlpp::Element *>(node);                                     \
                                                                                                             \
        if (!element)                                                                                        \
        {                                                                                                    \
            err = fmt::format("the type of the node '{0}' isn't xmlpp::Element.", node->get_name().c_str()); \
            return false;                                                                                    \
        }                                                                                                    \
    }

#define CONNECTION(text1, text2) text1##text2
#define CONNECT(text1, text2) CONNECTION(text1, text2)

class Defer
{
public:
    Defer(std::function<void(std::string)> func, std::string fun_name) : func_(func),
                                                                         fun_name_(fun_name) {}
    ~Defer() { func_(fun_name_); }

private:
    std::function<void(std::string)> func_;
    std::string fun_name_;
};

// helper macro for Defer class
#define SCOPE_EXIT(block) Defer CONNECT(_defer_, __LINE__)([&](std::string _arg_function) block, __FUNCTION__)

#define BREAK_IF_FALSE(cond) \
    {                        \
        if (!(cond)) break;  \
    }

#define BREAK_IF_TRUE(cond) \
    {                       \
        if (cond) break;    \
    }

#define RETURN_VAL_IF_FALSE(cond, val)             \
    {                                              \
        if (!(cond))                               \
        {                                          \
            KLOG_DEBUG("The condition is false."); \
            return val;                            \
        }                                          \
    }

#define RETURN_VAL_IF_TRUE(cond, val) \
    {                                 \
        if (cond) return val;         \
    }

#define RETURN_IF_FALSE(cond)                      \
    {                                              \
        if (!(cond))                               \
        {                                          \
            KLOG_DEBUG("The condition is false."); \
            return;                                \
        }                                          \
    }

#define RETURN_IF_TRUE(cond) \
    {                        \
        if (cond) return;    \
    }

#define CONTINUE_IF_FALSE(cond) \
    {                           \
        if (!(cond)) continue;  \
    }

#define CONTINUE_IF_TRUE(cond) \
    {                          \
        if (cond) continue;    \
    }

#define IGNORE_EXCEPTION(expr)          \
    {                                   \
        try                             \
        {                               \
            expr;                       \
        }                               \
        catch (const Glib::Error &e)    \
        {                               \
        }                               \
        catch (const std::exception &e) \
        {                               \
        }                               \
    }

#define POINTER_TO_STRING(p) ((p) ? p : std::string())

using StringHash = uint32_t;

constexpr StringHash prime = 9973;
constexpr StringHash basis = 0xCBF29CE4ul;
constexpr StringHash hash_compile_time(char const *str, StringHash last_value = basis)
{
    return *str ? hash_compile_time(str + 1, (StringHash)((*str ^ last_value) * (uint64_t)prime)) : last_value;
}

inline StringHash shash(char const *str)
{
    StringHash ret{basis};

    while (*str)
    {
        ret ^= *str;
        ret *= prime;
        str++;
    }

    return ret;
}

/// compile-time hash of string.
/// usage: "XXX"_hash
constexpr StringHash operator"" _hash(char const *p, size_t)
{
    return hash_compile_time(p);
}

#define KLOG_LEVEL_CLASSIFICATION(logLevel, pluginName, format, ...)     \
    do                                                                   \
    {                                                                    \
        klog_gtk3_append(logLevel, __FILENAME__, __FUNCTION__, __LINE__, \
                         pluginName " " format, ##__VA_ARGS__);          \
    } while (0)

#define KLOG_DEBUG_PLUGIN(pluginName, format, ...) \
    KLOG_LEVEL_CLASSIFICATION(G_LOG_LEVEL_DEBUG, pluginName, format, ##__VA_ARGS__)

#define KLOG_WARNING_PLUGIN(pluginName, format, ...) \
    KLOG_LEVEL_CLASSIFICATION(G_LOG_LEVEL_WARNING, pluginName, format, ##__VA_ARGS__)

#define KLOG_ERROR_PLUGIN(pluginName, format, ...) \
    KLOG_LEVEL_CLASSIFICATION(G_LOG_LEVEL_ERROR, pluginName, format, ##__VA_ARGS__)

#define KLOG_INFO_PLUGIN(pluginName, format, ...) \
    KLOG_LEVEL_CLASSIFICATION(G_LOG_LEVEL_INFO, pluginName, format, ##__VA_ARGS__)

#define KLOG_DEBUG_ACCOUNTS(format, ...) \
    KLOG_DEBUG_PLUGIN("ACCOUNTS", format, ##__VA_ARGS__)

#define KLOG_DEBUG_APPEARANCE(format, ...) \
    KLOG_DEBUG_PLUGIN("APPEARANCE", format, ##__VA_ARGS__)

#define KLOG_DEBUG_AUDIO(format, ...) \
    KLOG_DEBUG_PLUGIN("AUDIO", format, ##__VA_ARGS__)

#define KLOG_DEBUG_BLUETOOTH(format, ...) \
    KLOG_DEBUG_PLUGIN("BLUETOOTH", format, ##__VA_ARGS__)

#define KLOG_DEBUG_CLIPBOARD(format, ...) \
    KLOG_DEBUG_PLUGIN("CLIPBOARD", format, ##__VA_ARGS__)

#define KLOG_DEBUG_DISPLAY(format, ...) \
    KLOG_DEBUG_PLUGIN("DISPLAY", format, ##__VA_ARGS__)

#define KLOG_DEBUG_GREETER(format, ...) \
    KLOG_DEBUG_PLUGIN("GREETER", format, ##__VA_ARGS__)

#define KLOG_DEBUG_INPUTDEVICES(format, ...) \
    KLOG_DEBUG_PLUGIN("INPUTDEVICES", format, ##__VA_ARGS__)

#define KLOG_DEBUG_KEYBINDING(format, ...) \
    KLOG_DEBUG_PLUGIN("KEYBINDING", format, ##__VA_ARGS__)

#define KLOG_DEBUG_NETWORK(format, ...) \
    KLOG_DEBUG_PLUGIN("NETWORK", format, ##__VA_ARGS__)

#define KLOG_DEBUG_POWER(format, ...) \
    KLOG_DEBUG_PLUGIN("POWER", format, ##__VA_ARGS__)

#define KLOG_DEBUG_SYSTEMINFO(format, ...) \
    KLOG_DEBUG_PLUGIN("SYSTEMINFO", format, ##__VA_ARGS__)

#define KLOG_DEBUG_TIMEDATE(format, ...) \
    KLOG_DEBUG_PLUGIN("TIMEDATE", format, ##__VA_ARGS__)

#define KLOG_DEBUG_XSETTINGS(format, ...) \
    KLOG_DEBUG_PLUGIN("XSETTINGS", format, ##__VA_ARGS__)

#define KLOG_WARNING_ACCOUNTS(format, ...) \
    KLOG_WARNING_PLUGIN("ACCOUNTS", format, ##__VA_ARGS__)

#define KLOG_WARNING_APPEARANCE(format, ...) \
    KLOG_WARNING_PLUGIN("APPEARANCE", format, ##__VA_ARGS__)

#define KLOG_WARNING_AUDIO(format, ...) \
    KLOG_WARNING_PLUGIN("AUDIO", format, ##__VA_ARGS__)

#define KLOG_WARNING_BLUETOOTH(format, ...) \
    KLOG_WARNING_PLUGIN("BLUETOOTH", format, ##__VA_ARGS__)

#define KLOG_WARNING_CLIPBOARD(format, ...) \
    KLOG_WARNING_PLUGIN("CLIPBOARD", format, ##__VA_ARGS__)

#define KLOG_WARNING_DISPLAY(format, ...) \
    KLOG_WARNING_PLUGIN("DISPLAY", format, ##__VA_ARGS__)

#define KLOG_WARNING_GREETER(format, ...) \
    KLOG_WARNING_PLUGIN("GREETER", format, ##__VA_ARGS__)

#define KLOG_WARNING_INPUTDEVICES(format, ...) \
    KLOG_WARNING_PLUGIN("INPUTDEVICES", format, ##__VA_ARGS__)

#define KLOG_WARNING_KEYBINDING(format, ...) \
    KLOG_WARNING_PLUGIN("KEYBINDING", format, ##__VA_ARGS__)

#define KLOG_WARNING_NETWORK(format, ...) \
    KLOG_WARNING_PLUGIN("NETWORK", format, ##__VA_ARGS__)

#define KLOG_WARNING_POWER(format, ...) \
    KLOG_WARNING_PLUGIN("POWER", format, ##__VA_ARGS__)

#define KLOG_WARNING_SYSTEMINFO(format, ...) \
    KLOG_WARNING_PLUGIN("SYSTEMINFO", format, ##__VA_ARGS__)

#define KLOG_WARNING_TIMEDATE(format, ...) \
    KLOG_WARNING_PLUGIN("TIMEDATE", format, ##__VA_ARGS__)

#define KLOG_WARNING_XSETTINGS(format, ...) \
    KLOG_WARNING_PLUGIN("XSETTINGS", format, ##__VA_ARGS__)

#define KLOG_ERROR_ACCOUNTS(format, ...) \
    KLOG_ERROR_PLUGIN("ACCOUNTS", format, ##__VA_ARGS__)

#define KLOG_ERROR_APPEARANCE(format, ...) \
    KLOG_ERROR_PLUGIN("APPEARANCE", format, ##__VA_ARGS__)

#define KLOG_ERROR_AUDIO(format, ...) \
    KLOG_ERROR_PLUGIN("AUDIO", format, ##__VA_ARGS__)

#define KLOG_ERROR_BLUETOOTH(format, ...) \
    KLOG_ERROR_PLUGIN("BLUETOOTH", format, ##__VA_ARGS__)

#define KLOG_ERROR_CLIPBOARD(format, ...) \
    KLOG_ERROR_PLUGIN("CLIPBOARD", format, ##__VA_ARGS__)

#define KLOG_ERROR_DISPLAY(format, ...) \
    KLOG_ERROR_PLUGIN("DISPLAY", format, ##__VA_ARGS__)

#define KLOG_ERROR_GREETER(format, ...) \
    KLOG_ERROR_PLUGIN("GREETER", format, ##__VA_ARGS__)

#define KLOG_ERROR_INPUTDEVICES(format, ...) \
    KLOG_ERROR_PLUGIN("INPUTDEVICES", format, ##__VA_ARGS__)

#define KLOG_ERROR_KEYBINDING(format, ...) \
    KLOG_ERROR_PLUGIN("KEYBINDING", format, ##__VA_ARGS__)

#define KLOG_ERROR_NETWORK(format, ...) \
    KLOG_ERROR_PLUGIN("NETWORK", format, ##__VA_ARGS__)

#define KLOG_ERROR_POWER(format, ...) \
    KLOG_ERROR_PLUGIN("POWER", format, ##__VA_ARGS__)

#define KLOG_ERROR_SYSTEMINFO(format, ...) \
    KLOG_ERROR_PLUGIN("SYSTEMINFO", format, ##__VA_ARGS__)

#define KLOG_ERROR_TIMEDATE(format, ...) \
    KLOG_ERROR_PLUGIN("TIMEDATE", format, ##__VA_ARGS__)

#define KLOG_ERROR_XSETTINGS(format, ...) \
    KLOG_ERROR_PLUGIN("XSETTINGS", format, ##__VA_ARGS__)

#define KLOG_INFO_ACCOUNTS(format, ...) \
    KLOG_INFO_PLUGIN("ACCOUNTS", format, ##__VA_ARGS__)

#define KLOG_INFO_APPEARANCE(format, ...) \
    KLOG_INFO_PLUGIN("APPEARANCE", format, ##__VA_ARGS__)

#define KLOG_INFO_AUDIO(format, ...) \
    KLOG_INFO_PLUGIN("AUDIO", format, ##__VA_ARGS__)

#define KLOG_INFO_BLUETOOTH(format, ...) \
    KLOG_INFO_PLUGIN("BLUETOOTH", format, ##__VA_ARGS__)

#define KLOG_INFO_CLIPBOARD(format, ...) \
    KLOG_INFO_PLUGIN("CLIPBOARD", format, ##__VA_ARGS__)

#define KLOG_INFO_DISPLAY(format, ...) \
    KLOG_INFO_PLUGIN("DISPLAY", format, ##__VA_ARGS__)

#define KLOG_INFO_GREETER(format, ...) \
    KLOG_INFO_PLUGIN("GREETER", format, ##__VA_ARGS__)

#define KLOG_INFO_INPUTDEVICES(format, ...) \
    KLOG_INFO_PLUGIN("INPUTDEVICES", format, ##__VA_ARGS__)

#define KLOG_INFO_KEYBINDING(format, ...) \
    KLOG_INFO_PLUGIN("KEYBINDING", format, ##__VA_ARGS__)

#define KLOG_INFO_NETWORK(format, ...) \
    KLOG_INFO_PLUGIN("NETWORK", format, ##__VA_ARGS__)

#define KLOG_INFO_POWER(format, ...) \
    KLOG_INFO_PLUGIN("POWER", format, ##__VA_ARGS__)

#define KLOG_INFO_SYSTEMINFO(format, ...) \
    KLOG_INFO_PLUGIN("SYSTEMINFO", format, ##__VA_ARGS__)

#define KLOG_INFO_TIMEDATE(format, ...) \
    KLOG_INFO_PLUGIN("TIMEDATE", format, ##__VA_ARGS__)

#define KLOG_INFO_XSETTINGS(format, ...) \
    KLOG_INFO_PLUGIN("XSETTINGS", format, ##__VA_ARGS__)

}  // namespace Kiran
