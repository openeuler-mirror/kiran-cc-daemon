/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
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

#include "plugins/power/wrapper/power-profiles.h"

namespace Kiran
{
class PowerProfilesTuned : public PowerProfiles
{
public:
    PowerProfilesTuned();
    virtual ~PowerProfilesTuned(){};

    virtual void init();
    virtual bool switch_profile(int32_t profile_mode);
    // 这里永远返回0，因为不支持hold操作
    virtual uint32_t hold_profile(int32_t profile_mode, const std::string& reason);
    // 不支持该操作，什么都不执行
    virtual void release_profile(uint32_t cookie){};
    virtual int32_t get_active_profile();

private:
    std::string porfile_mode_enum2str(int32_t profile_mode);
    int32_t porfile_mode_str2enum(const std::string& profile_mode_str);
    void on_profile_signal(const Glib::ustring& sender_name,
                           const Glib::ustring& signal_name,
                           const Glib::VariantContainerBase& parameters);

private:
    Glib::RefPtr<Gio::DBus::Proxy> profiles_proxy_;
};
}  // namespace Kiran
