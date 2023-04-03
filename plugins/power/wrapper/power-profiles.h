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

#include "lib/base/base.h"

namespace Kiran
{
#define POWER_PROFILE_SAVER "power-saver"
#define POWER_PROFILE_BALANCED "balanced"
#define POWER_PROFILE_PERFORMANCE "performance"

class PowerProfiles
{
public:
    PowerProfiles();
    virtual ~PowerProfiles(){};

    void init();

    // 设置模式，如果调用了ReleaseProfile，则进行恢复。如果有其他用户进行了手动设置（直接修改ActiveProfile属性），则不再hold当前模式
    uint32_t hold_profile(const std::string &profile,
                          const std::string &reason,
                          const std::string &application_id);

    // 释放hold_profile操作。恢复到之前的模式
    void release_profile(uint32_t cookie);

    std::string get_active_profile();

    sigc::signal<void, const Glib::ustring &> &signal_active_profile_changed() { return this->active_profile_changed_; };

private:
    void on_properties_changed(const Gio::DBus::Proxy::MapChangedProperties &changed_properties,
                               const std::vector<Glib::ustring> &invalidated_properties);

private:
    Glib::RefPtr<Gio::DBus::Proxy> profiles_proxy_;

    sigc::signal<void, const Glib::ustring &> active_profile_changed_;
};
}  // namespace Kiran
