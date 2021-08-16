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

#include "plugins/accounts/user-cache.h"

#include "plugins/accounts/user.h"

namespace Kiran
{
#define USERDIR "/var/lib/AccountsService/users"

UserCache::UserCache(std::shared_ptr<User> user) : user_(user)
{
    this->keyfile_ = std::make_shared<Glib::KeyFile>();
    this->load_cache_file();
}

UserCache::~UserCache()
{
}

std::string UserCache::get_string(const std::string &group_name, const std::string &key)
{
    std::string result;
    IGNORE_EXCEPTION({
        result = this->keyfile_->get_string(group_name, key);
    });
    return result;
}

bool UserCache::get_boolean(const std::string &group_name, const std::string &key)
{
    bool result = false;
    IGNORE_EXCEPTION({
        result = this->keyfile_->get_boolean(group_name, key);
    });
    return result;
}

int32_t UserCache::get_int(const std::string &group_name, const std::string &key)
{
    int32_t result = false;
    IGNORE_EXCEPTION({
        result = this->keyfile_->get_integer(group_name, key);
    });
    return result;
}

VPSS UserCache::get_group_kv(const std::string &group_name)
{
    VPSS result;

    try
    {
        for (const auto &key : this->keyfile_->get_keys(group_name))
        {
            auto value = this->keyfile_->get_string(group_name, key);
            result.push_back(std::make_pair(key, value));
        }
    }
    catch (const Glib::Exception &e)
    {
        // 忽略错误
    }
    return result;
}

bool UserCache::set_value(const std::string &group_name,
                          const std::string &key,
                          bool value)
{
    this->keyfile_->set_boolean(group_name, key, value);
    return this->save_cache_file();
}

bool UserCache::set_value(const std::string &group_name,
                          const std::string &key,
                          const std::string &value)
{
    this->keyfile_->set_string(group_name, key, value);
    return this->save_cache_file();
}

bool UserCache::set_value(const std::string &group_name, const std::string &key, int32_t value)
{
    this->keyfile_->set_integer(group_name, key, value);
    return this->save_cache_file();
}

bool UserCache::remove_key(const std::string &group_name, const std::string &key)
{
    IGNORE_EXCEPTION({
        this->keyfile_->remove_key(group_name, key);
        return this->save_cache_file();
    });
    return true;
}

bool UserCache::load_cache_file()
{
    auto user = this->user_.lock();
    RETURN_VAL_IF_FALSE(user, false);

    // 非root的系统用户不缓存数据
    RETURN_VAL_IF_TRUE(user->system_account_get() && user->uid_get() != 0, false);

    auto filename = Glib::build_filename(USERDIR, user->user_name_get());
    try
    {
        this->keyfile_->load_from_file(filename);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("failed to load file %s: %s.", filename.c_str(), e.what().c_str());
        return false;
    }
    return true;
}

bool UserCache::save_cache_file()
{
    KLOG_PROFILE("");

    auto user = this->user_.lock();
    RETURN_VAL_IF_FALSE(user, false);

    // 非root的系统用户不缓存数据
    RETURN_VAL_IF_TRUE(user->system_account_get() && user->uid_get() != 0, false);

    try
    {
        auto filename = Glib::build_filename(USERDIR, user->user_name_get());
        return this->keyfile_->save_to_file(filename);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Saving data for user %s failed: %s", user->user_name_get().c_str(), e.what().c_str());
        return false;
    }
    return true;
}

}  // namespace Kiran