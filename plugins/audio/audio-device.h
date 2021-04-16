/**
 * @file          /kiran-cc-daemon/plugins/audio/audio-device.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

#include <audio_device_dbus_stub.h>
#include "plugins/audio/pulse/pulse-device.h"

namespace Kiran
{
class AudioDevice : public SessionDaemon::Audio::DeviceStub
{
public:
    AudioDevice(std::shared_ptr<PulseDevice> device);
    virtual ~AudioDevice();

    bool init(const std::string &object_path_prefix);

    std::string get_object_path() { return this->object_path_; };

    virtual guint32 index_get() { return this->device_->get_index(); };
    virtual Glib::ustring name_get() { return this->device_->get_name(); };
    virtual bool mute_get() { return this->device_->get_mute(); };
    virtual double volume_get();
    virtual double balance_get() { return this->device_->get_balance(); }
    virtual double fade_get() { return this->device_->get_balance(); }
    virtual double base_volume_get();
    virtual guint32 card_index_get() { return this->device_->get_card_index(); };
    virtual Glib::ustring active_port_get() { return this->device_->get_active_port(); };
    virtual guint32 state_get() { return this->device_->get_flags(); };

protected:
    // 设置正在使用的端口
    virtual void SetActivePort(const Glib::ustring &name, MethodInvocation &invocation);
    // 获取所有绑定端口
    virtual void GetPorts(MethodInvocation &invocation);
    // 设置音量
    virtual void SetVolume(double volume, MethodInvocation &invocation);
    // 设置左声道和右声道的平衡
    virtual void SetBalance(double balance, MethodInvocation &invocation);
    // 设置远声道和近声道的平衡
    virtual void SetFade(double fade, MethodInvocation &invocation);
    // 设置静音
    virtual void SetMute(bool mute, MethodInvocation &invocation);

    virtual bool index_setHandler(guint32 value) { return true; };
    virtual bool name_setHandler(const Glib::ustring &value) { return true; };
    virtual bool mute_setHandler(bool value) { return this->device_->set_mute(value); };
    virtual bool volume_setHandler(double value);
    virtual bool balance_setHandler(double value) { return this->device_->set_balance(value); };
    virtual bool fade_setHandler(double value) { return this->device_->set_fade(value); };
    virtual bool base_volume_setHandler(double value) { return true; };
    virtual bool card_index_setHandler(guint32 value) { return true; };
    virtual bool active_port_setHandler(const Glib::ustring &value) { return this->device_->set_active_port(value.raw()); };
    virtual bool state_setHandler(guint32 value) { return true; }

private:
    bool dbus_register();
    void dbus_unregister();

    void on_node_info_changed_cb(PulseNodeField field);
    void on_active_port_changed_cb(const std::string &active_port_name);

private:
    std::shared_ptr<PulseDevice> device_;

    Glib::RefPtr<Gio::DBus::Connection> dbus_connect_;
    uint32_t object_register_id_;
    Glib::DBusObjectPathString object_path_;
};

}  // namespace Kiran
