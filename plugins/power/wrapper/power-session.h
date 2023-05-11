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

#include "lib/base/base.h"

namespace Kiran
{
// 对SessionManager的dbus接口的封装
class PowerSession
{
public:
    PowerSession();
    virtual ~PowerSession(){};

    void init();

    // 空闲状态发生变化
    sigc::signal<void, bool>& signal_idle_status_changed() { return this->idle_status_changed_; };
    // 禁用状态发生变化
    sigc::signal<void>& signal_inhibitor_changed() { return this->inhibitor_changed_; };

    // 获取空闲状态
    bool get_idle() { return this->is_idle_; };
    // 空闲状态是否禁用
    bool get_idle_inhibited() { return this->is_idle_inhibited_; };
    // 挂起状态是否禁用
    bool get_suspend_inhibited() { return this->is_suspend_inhibited_; };

    // 挂机
    bool can_suspend();
    void suspend();
    // 休眠
    bool can_hibernate();
    void hibernate();
    // 关机
    bool can_shutdown();
    void shutdown();

private:
    // 获取空闲状态
    uint32_t get_status();
    // 判断logout, switch-user, suspend和idle的禁用状态
    bool get_inhibited(uint32_t flag);

    void on_sm_signal(const Glib::ustring& sender_name, const Glib::ustring& signal_name, const Glib::VariantContainerBase& parameters);
    void on_sm_inhibitor_changed_cb(const Glib::VariantContainerBase& parameters);

    void on_sm_presence_signal(const Glib::ustring& sender_name, const Glib::ustring& signal_name, const Glib::VariantContainerBase& parameters);
    void on_sm_presence_status_changed_cb(const Glib::VariantContainerBase& parameters);

private:
    // 会话管理代理类(session manager proxy)
    Glib::RefPtr<Gio::DBus::Proxy> sm_proxy_;
    Glib::RefPtr<Gio::DBus::Proxy> sm_presence_proxy_;

    sigc::signal<void, bool> idle_status_changed_;
    sigc::signal<void> inhibitor_changed_;

    bool is_idle_;
    bool is_idle_inhibited_;
    bool is_suspend_inhibited_;
};
}  // namespace Kiran