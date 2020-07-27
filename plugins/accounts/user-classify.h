/*
 * @Author       : tangjie02
 * @Date         : 2020-07-23 14:29:58
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-24 13:44:45
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/user-classify.h
 */
#pragma once

#include <set>
#include <string>

namespace Kiran
{
class UserClassify
{
public:
    UserClassify(){};
    virtual ~UserClassify(){};

    static bool is_human(uint32_t uid,
                         const std::string &username,
                         const std::string &shell);

private:
    static bool is_invalid_shell(const std::string &shell);

private:
    static const std::set<std::string> default_excludes_;
};
}  // namespace Kiran