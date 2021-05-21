/**
 * @file          /kiran-cc-daemon/plugins/audio/audio-device.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/audio/audio-device.h"

#include <json/json.h>
#include "audio-i.h"
#include "plugins/audio/audio-utils.h"

namespace Kiran
{
AudioDevice::AudioDevice(std::shared_ptr<PulseDevice> device) : device_(device),
                                                                object_register_id_(0)
{
    this->device_->signal_node_info_changed().connect(sigc::mem_fun(this, &AudioDevice::on_node_info_changed_cb));
    this->device_->signal_active_port_changed().connect(sigc::mem_fun(this, &AudioDevice::on_active_port_changed_cb));
}

AudioDevice::~AudioDevice()
{
    this->dbus_unregister();
}

bool AudioDevice::init(const std::string &object_path_prefix)
{
    RETURN_VAL_IF_FALSE(this->device_, false);

    this->object_path_ = fmt::format("{0}{1}", object_path_prefix, this->device_->get_index());
    return this->dbus_register();
}

double AudioDevice::volume_get()
{
    auto volume = this->device_->get_volume();
    return AudioUtils::volume_absolute2range(volume, this->device_->get_min_volume(), this->device_->get_max_volume());
}

double AudioDevice::base_volume_get()
{
    auto volume = this->device_->get_base_volume();
    return AudioUtils::volume_absolute2range(volume, this->device_->get_min_volume(), this->device_->get_max_volume());
}

void AudioDevice::SetActivePort(const Glib::ustring &name, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("port name: %s.", name.c_str());

    if (!this->active_port_set(name))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_SET_SINK_ACTIVE_PORT_FAILED);
    }
    invocation.ret();
}

void AudioDevice::GetPorts(MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");

    Json::Value values;
    Json::FastWriter writer;

    auto ports = this->device_->get_ports();
    for (uint32_t i = 0; i < ports.size(); ++i)
    {
        values[i]["name"] = ports[i]->get_name();
        values[i]["description"] = ports[i]->get_description();
        values[i]["priority"] = ports[i]->get_priority();
    }

    auto result = writer.write(values);
    invocation.ret(result);
}

void AudioDevice::SetVolume(double volume, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("volume: %f.", volume);

    if (volume < 0 || volume > 1.0 + EPS)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_VOLUME_RANGE_INVLAID);
    }

    if (!this->volume_set(volume))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_SET_VOLUME_FAILED);
    }

    // 如果音量大于0，则取消静音
    if (volume > EPS)
    {
        this->mute_set(false);
    }
    invocation.ret();
}

void AudioDevice::SetBalance(double balance, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("balance: %f.", balance);

    if (balance < -1 || balance > 1)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_BALANCE_RANGE_INVLAID);
    }

    if (!this->balance_set(balance))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_SET_BALANCE_FAILED);
    }
    invocation.ret();
}

void AudioDevice::SetFade(double fade, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("fade: %f.", fade);

    if (fade < -1 || fade > 1)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_FADE_RANGE_INVLAID);
    }

    if (!this->fade_set(fade))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_SET_FADE_FAILED);
    }
    invocation.ret();
}

void AudioDevice::SetMute(bool mute, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("mute: %d.", mute);

    if (!this->mute_set(mute))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_SET_MUTE_FAILED);
    }

    // 如果设置了静音，则将音量也设置为0
    if (mute)
    {
        this->volume_set(0);
    }
    invocation.ret();
}

bool AudioDevice::volume_setHandler(double value)
{
    auto volume_absolute = AudioUtils::volume_range2absolute(value,
                                                             this->device_->get_min_volume(),
                                                             this->device_->get_max_volume());

    return this->device_->set_volume(volume_absolute);
}

bool AudioDevice::dbus_register()
{
    SETTINGS_PROFILE("register object path: %s.", this->object_path_.c_str());

    RETURN_VAL_IF_FALSE(this->device_, false);

    try
    {
        this->dbus_connect_ = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SESSION);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("Failed to get session bus: %s.", e.what().c_str());
        return false;
    }

    this->object_register_id_ = this->register_object(this->dbus_connect_, this->object_path_.c_str());
    return true;
}

void AudioDevice::dbus_unregister()
{
    SETTINGS_PROFILE("unregister object path: %s.", this->object_path_.c_str());

    if (this->object_register_id_)
    {
        this->unregister_object();
        this->object_register_id_ = 0;
    }
}

void AudioDevice::on_node_info_changed_cb(PulseNodeField field)
{
    // 这里的主要目的是为了触发dbus属性信号，小心使用，避免出现死循环
    switch (field)
    {
    case PulseNodeField::PULSE_NODE_FIELD_BALANCE:
        this->balance_set(this->balance_get());
        break;
    case PulseNodeField::PULSE_NODE_FIELD_FADE:
        this->fade_set(this->fade_get());
        break;
    case PulseNodeField::PULSE_NODE_FIELD_MUTE:
        this->mute_set(this->mute_get());
        break;
    case PulseNodeField::PULSE_NODE_FIELD_VOLUME:
        this->volume_set(this->volume_get());
        break;
    default:
        break;
    }
}

void AudioDevice::on_active_port_changed_cb(const std::string &active_port_name)
{
    // 这里的主要目的是为了触发dbus属性信号，小心使用，避免出现死循环
    this->active_port_set(active_port_name);
}

}  // namespace Kiran