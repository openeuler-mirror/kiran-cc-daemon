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

#include <QString>
#include <cstdio>
#include <functional>
#include <vector>

namespace Kiran
{
#define SYSTEMD_NAME "org.freedesktop.systemd1"
#define SYSTEMD_PATH "/org/freedesktop/systemd1"
#define SYSTEMD_MANAGER_INTERFACE "org.freedesktop.systemd1.Manager"
#define SYSTEMD_UNIT_INTERFACE "org.freedesktop.systemd1.Unit"

#define POLKIT_NAME "org.freedesktop.PolicyKit1"
#define POLKIT_PATH "/org/freedesktop/PolicyKit1/Authority"
#define POLKIT_INTERFACE "org.freedesktop.PolicyKit1.Authority"

#define EPS 1e-4

#define CHECK_XMLPP_ELEMENT(node, error)                                                                    \
    {                                                                                                       \
        const auto element = dynamic_cast<const xmlpp::Element *>(node);                                    \
                                                                                                            \
        if (!element)                                                                                       \
        {                                                                                                   \
            error = QString("the type of the node %1 isn't xmlpp::Element.").arg(node->get_name().c_str()); \
            return false;                                                                                   \
        }                                                                                                   \
    }

#define CONNECTION(text1, text2) text1##text2
#define CONNECT(text1, text2) CONNECTION(text1, text2)

class KCDDefer
{
public:
    KCDDefer(std::function<void(const QString &)> func, const QString &fun_name) : m_function(func),
                                                                                   m_functionName(fun_name) {}
    ~KCDDefer() { m_function(m_functionName); }

private:
    std::function<void(const QString &)> m_function;
    QString m_functionName;
};

// helper macro for KCDDefer class
#define SCOPE_EXIT(block) KCDDefer CONNECT(_defer_, __LINE__)([&](const QString &argFunction) block, __FUNCTION__)

#define BREAK_IF_FALSE(cond) \
    {                        \
        if (!(cond)) break;  \
    }

#define BREAK_IF_TRUE(cond) \
    {                       \
        if (cond) break;    \
    }

#define RETURN_VAL_IF_FALSE(cond, val) \
    {                                  \
        if (!(cond))                   \
        {                              \
            return val;                \
        }                              \
    }

#define RETURN_VAL_IF_TRUE(cond, val) \
    {                                 \
        if (cond) return val;         \
    }

#define RETURN_IF_FALSE(cond) \
    {                         \
        if (!(cond))          \
        {                     \
            return;           \
        }                     \
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

#define POINTER_TO_STRING(p) ((p) ? QString(p) : QString())

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

#define XCB_REPLY_CONNECTION_ARG(connection, ...) connection

struct StdFreeDeleter
{
    void operator()(void *p) const noexcept { return std::free(p); }
};

#define XCB_REPLY(call, ...)                         \
    std::unique_ptr<call##_reply_t, StdFreeDeleter>( \
        call##_reply(XCB_REPLY_CONNECTION_ARG(__VA_ARGS__), call(__VA_ARGS__), nullptr))

#define XCB_REPLY_UNCHECKED(call, ...)               \
    std::unique_ptr<call##_reply_t, StdFreeDeleter>( \
        call##_reply(XCB_REPLY_CONNECTION_ARG(__VA_ARGS__), call##_unchecked(__VA_ARGS__), nullptr))

}  // namespace Kiran
