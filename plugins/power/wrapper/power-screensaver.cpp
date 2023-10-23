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

#include "plugins/power/wrapper/power-screensaver.h"

namespace Kiran
{
#define SCREENSAVER_DBUS_NAME "com.kylinsec.Kiran.ScreenSaver"
#define SCREENSAVER_DBUS_OBJECT_PATH "/com/kylinsec/Kiran/ScreenSaver"
#define SCREENSAVER_DBUS_INTERFACE "com.kylinsec.Kiran.ScreenSaver"

// 屏保锁屏后，检查锁屏状态的最大次数
#define SCREENSAVER_LOCK_CHECK_MAX_COUNT 50

PowerScreenSaver::PowerScreenSaver()
{
}

void PowerScreenSaver::init()
{
    try
    {
        this->screensaver_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION,
                                                                         SCREENSAVER_DBUS_NAME,
                                                                         SCREENSAVER_DBUS_OBJECT_PATH,
                                                                         SCREENSAVER_DBUS_INTERFACE);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
        return;
    }
}

bool PowerScreenSaver::lock()
{
    RETURN_VAL_IF_FALSE(this->screensaver_proxy_, false);

    // Lock函数无返回，因此这里使用异步调用后再循环检查屏幕保护程序是否已经启动
    try
    {
        this->screensaver_proxy_->call("Lock", Gio::SlotAsyncReady(), Glib::VariantContainerBase());
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
        return false;
    }

    bool is_running = false;
    for (int32_t i = 0; i < SCREENSAVER_LOCK_CHECK_MAX_COUNT; ++i)
    {
        // 如果屏幕保护程序已经运行，则停止检查
        if (this->check_running())
        {
            is_running = true;
            break;
        }
        KLOG_DEBUG_POWER("Timeout waiting for screensaver");
        g_usleep(1000 * 100);
    }

    if (!is_running)
    {
        KLOG_WARNING_POWER("Failed to lock screen.");
        return false;
    }

    return true;
}

uint32_t PowerScreenSaver::add_throttle(const std::string& reason)
{
    RETURN_VAL_IF_FALSE(this->screensaver_proxy_, 0);

    auto parameters = g_variant_new("(ss)", "Power screensaver", reason.c_str());
    Glib::VariantContainerBase base(parameters, false);
    Glib::VariantContainerBase retval;

    try
    {
        retval = this->screensaver_proxy_->call_sync("Throttle", base);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
        return 0;
    }

    try
    {
        auto v1 = retval.get_child(0);
        auto cookie = Glib::VariantBase::cast_dynamic<Glib::Variant<uint32_t>>(v1).get();
        KLOG_DEBUG_POWER("The Cookie is %u.", cookie);
        return cookie;
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING_POWER("%s", e.what());
    }
    return 0;
}

uint32_t PowerScreenSaver::lock_and_throttle(const std::string& reason)
{
    KLOG_DEBUG_POWER("Lock and throttle");

    RETURN_VAL_IF_FALSE(this->lock(), 0);
    return this->add_throttle(reason);
}

bool PowerScreenSaver::remove_throttle(uint32_t cookie)
{
    KLOG_DEBUG_POWER("Remove throttle");
    RETURN_VAL_IF_FALSE(this->screensaver_proxy_, false);

    auto parameters = g_variant_new("(u)", cookie);
    Glib::VariantContainerBase base(parameters, false);

    try
    {
        this->screensaver_proxy_->call_sync("UnThrottle", base);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
        return false;
    }

    return true;
}

bool PowerScreenSaver::poke()
{
    RETURN_VAL_IF_FALSE(this->screensaver_proxy_, false);

    try
    {
        this->screensaver_proxy_->call("SimulateUserActivity", Gio::SlotAsyncReady(), Glib::VariantContainerBase());
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
        return false;
    }

    return true;
}

bool PowerScreenSaver::check_running()
{
    Glib::VariantContainerBase retval;

    RETURN_VAL_IF_FALSE(this->screensaver_proxy_, false);

    try
    {
        retval = this->screensaver_proxy_->call_sync("GetActive", Glib::VariantContainerBase());
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
        return false;
    }

    try
    {
        auto v1 = retval.get_child(0);
        auto actived = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(v1).get();
        return actived;
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING_POWER("%s", e.what());
    }
    return false;
}

}  // namespace Kiran