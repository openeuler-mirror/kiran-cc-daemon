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

#include "plugins/audio/audio-device.h"

#include <json/json.h>
#include "audio-i.h"
#include "plugins/audio/audio-utils.h"

namespace Kiran
{
AudioDevice::AudioDevice(std::shared_ptr<PulseDevice> device) : device_(device),
                                                                object_register_id_(0)
{
    this->mute_ = this->device_->get_mute();
    this->volume_ = AudioUtils::volume_absolute2range(this->device_->get_volume(),
                                                      this->device_->get_min_volume(),
                                                      this->device_->get_max_volume());
    this->balance_ = this->device_->get_balance();
    this->fade_ = this->device_->get_fade();
    this->active_port_ = this->device_->get_active_port();

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

double AudioDevice::base_volume_get()
{
    auto volume = this->device_->get_base_volume();
    return AudioUtils::volume_absolute2range(volume, this->device_->get_min_volume(), this->device_->get_max_volume());
}

void AudioDevice::SetActivePort(const Glib::ustring &name, MethodInvocation &invocation)
{
    KLOG_DEBUG_AUDIO("Set %s as active port.", name.c_str());

    if (!this->device_->set_active_port(name))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_SET_SINK_ACTIVE_PORT_FAILED);
    }
    invocation.ret();
}

void AudioDevice::GetPorts(MethodInvocation &invocation)
{
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
    if (volume < 0 || volume > 1.0 + EPS)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_VOLUME_RANGE_INVLAID);
    }

    auto volume_absolute = AudioUtils::volume_range2absolute(volume,
                                                             this->device_->get_min_volume(),
                                                             this->device_->get_max_volume());

    if (!this->device_->set_volume(volume_absolute))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_SET_VOLUME_FAILED);
    }

    // 如果音量大于0，则取消静音
    if (volume > EPS)
    {
        this->device_->set_mute(false);
    }
    invocation.ret();
}

void AudioDevice::SetBalance(double balance, MethodInvocation &invocation)
{
    if (balance < -1 || balance > 1)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_BALANCE_RANGE_INVLAID);
    }

    if (!this->device_->set_balance(balance))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_SET_BALANCE_FAILED);
    }
    invocation.ret();
}

void AudioDevice::SetFade(double fade, MethodInvocation &invocation)
{
    if (fade < -1 || fade > 1)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_FADE_RANGE_INVLAID);
    }

    if (!this->device_->set_fade(fade))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_SET_FADE_FAILED);
    }
    invocation.ret();
}

void AudioDevice::SetMute(bool mute, MethodInvocation &invocation)
{
    if (!this->device_->set_mute(mute))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_SET_MUTE_FAILED);
    }

    invocation.ret();
}

void AudioDevice::GetProperty(const Glib::ustring &key, MethodInvocation &invocation)
{
    auto value = this->device_->get_property(key);
    invocation.ret(value);
}

#define SET_COMMON_PROPERTY(property, type)                    \
    bool AudioDevice::property##_setHandler(type value)        \
    {                                                          \
        RETURN_VAL_IF_TRUE(this->property##_ == value, false); \
        this->property##_ = value;                             \
        return true;                                           \
    }

#define SET_DOUBLE_PROPERTY(property)                     \
    bool AudioDevice::property##_setHandler(double value) \
    {                                                     \
        if (std::fabs(this->property##_ - value) < EPS)   \
        {                                                 \
            return false;                                 \
        }                                                 \
        this->property##_ = value;                        \
        return true;                                      \
    }

SET_COMMON_PROPERTY(mute, bool)
SET_DOUBLE_PROPERTY(volume)
SET_DOUBLE_PROPERTY(balance)
SET_DOUBLE_PROPERTY(fade)
SET_COMMON_PROPERTY(active_port, const Glib::ustring &)

bool AudioDevice::dbus_register()
{
    KLOG_DEBUG_AUDIO("register object path: %s.", this->object_path_.c_str());
    RETURN_VAL_IF_FALSE(this->device_, false);

    try
    {
        this->dbus_connect_ = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SESSION);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_AUDIO("Failed to get session bus: %s.", e.what().c_str());
        return false;
    }

    this->object_register_id_ = this->register_object(this->dbus_connect_, this->object_path_.c_str());
    return true;
}

void AudioDevice::dbus_unregister()
{
    if (this->object_register_id_)
    {
        this->unregister_object();
        this->object_register_id_ = 0;
    }
}

void AudioDevice::on_node_info_changed_cb(PulseNodeField field)
{
    switch (field)
    {
    case PulseNodeField::PULSE_NODE_FIELD_BALANCE:
        this->balance_set(this->device_->get_balance());
        break;
    case PulseNodeField::PULSE_NODE_FIELD_FADE:
        this->fade_set(this->device_->get_fade());
        break;
    case PulseNodeField::PULSE_NODE_FIELD_MUTE:
        this->mute_set(this->device_->get_mute());
        break;
    case PulseNodeField::PULSE_NODE_FIELD_VOLUME:
    {
        auto volume_range = AudioUtils::volume_absolute2range(this->device_->get_volume(),
                                                              this->device_->get_min_volume(),
                                                              this->device_->get_max_volume());
        this->volume_set(volume_range);
        break;
    }
    default:
        break;
    }
}

void AudioDevice::on_active_port_changed_cb(const std::string &active_port_name)
{
    this->active_port_set(active_port_name);
}

}  // namespace Kiran