/**
 * @file          /kiran-cc-daemon/plugins/power/wrapper/power-screensaver.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/power/wrapper/power-screensaver.h"

namespace Kiran
{
#define SCREENSAVER_DBUS_NAME "org.mate.ScreenSaver"
#define SCREENSAVER_DBUS_OBJECT_PATH "/org/mate/ScreenSaver"
#define SCREENSAVER_DBUS_INTERFACE "org.mate.ScreenSaver"

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
        LOG_WARNING("%s", e.what().c_str());
        return;
    }
}

bool PowerScreenSaver::lock()
{
    SETTINGS_PROFILE("");
    RETURN_VAL_IF_FALSE(this->screensaver_proxy_, false);

    // Lock函数无返回，因此这里使用异步调用后再循环检查屏幕保护程序是否已经启动
    try
    {
        this->screensaver_proxy_->call("Lock", Gio::SlotAsyncReady(), Glib::VariantContainerBase());
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("%s", e.what().c_str());
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
        LOG_DEBUG("timeout waiting for screensaver");
        g_usleep(1000 * 100);
    }

    if (!is_running)
    {
        LOG_WARNING("Failed to lock screen.");
        return false;
    }

    return true;
}

uint32_t PowerScreenSaver::add_throttle(const std::string& reason)
{
    SETTINGS_PROFILE("reason: %s.", reason.c_str());
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
        LOG_WARNING("%s", e.what().c_str());
        return 0;
    }

    try
    {
        auto v1 = retval.get_child(0);
        auto cookie = Glib::VariantBase::cast_dynamic<Glib::Variant<uint32_t>>(v1).get();
        LOG_DEBUG("cookie: %u.", cookie);
        return cookie;
    }
    catch (const std::exception& e)
    {
        LOG_WARNING("%s", e.what());
    }
    return 0;
}

uint32_t PowerScreenSaver::lock_and_throttle(const std::string& reason)
{
    SETTINGS_PROFILE("reason: %s.", reason.c_str());

    RETURN_VAL_IF_FALSE(this->lock(), 0);
    return this->add_throttle(reason);
}

bool PowerScreenSaver::remove_throttle(uint32_t cookie)
{
    SETTINGS_PROFILE("cookie: %u", cookie);
    RETURN_VAL_IF_FALSE(this->screensaver_proxy_, false);

    auto parameters = g_variant_new("(u)", cookie);
    Glib::VariantContainerBase base(parameters, false);

    try
    {
        this->screensaver_proxy_->call_sync("UnThrottle", base);
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("%s", e.what());
        return false;
    }

    return true;
}

bool PowerScreenSaver::poke()
{
    SETTINGS_PROFILE("");
    RETURN_VAL_IF_FALSE(this->screensaver_proxy_, false);

    try
    {
        this->screensaver_proxy_->call("SimulateUserActivity", Gio::SlotAsyncReady(), Glib::VariantContainerBase());
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("%s", e.what());
        return false;
    }

    return true;
}

bool PowerScreenSaver::check_running()
{
    SETTINGS_PROFILE("");
    Glib::VariantContainerBase retval;

    RETURN_VAL_IF_FALSE(this->screensaver_proxy_, false);

    try
    {
        retval = this->screensaver_proxy_->call_sync("GetActive", Glib::VariantContainerBase());
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("%s", e.what().c_str());
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
        LOG_WARNING("%s", e.what());
    }
    return false;
}

}  // namespace Kiran