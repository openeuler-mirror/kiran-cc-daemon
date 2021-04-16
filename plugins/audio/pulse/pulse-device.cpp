/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-device.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/audio/pulse/pulse-device.h"

namespace Kiran
{
PulseDeviceInfo::PulseDeviceInfo(const pa_sink_info *sink_info) : index(sink_info->index),
                                                                  name(POINTER_TO_STRING(sink_info->name)),
                                                                  channel_map(sink_info->channel_map),
                                                                  cvolume(sink_info->volume),
                                                                  mute(sink_info->mute),
                                                                  base_volume(sink_info->base_volume),
                                                                  card_index(sink_info->card)
{
    for (uint32_t i = 0; i < sink_info->n_ports; ++i)
    {
        auto port = std::make_shared<PulsePort>(sink_info->ports[i]);
        auto iter = this->ports.emplace(port->get_name(), port);
        if (!iter.second)
        {
            LOG_WARNING("The port %s already exist.", port->get_name().c_str());
        }
    }

    if (sink_info->active_port)
    {
        this->active_port_name = POINTER_TO_STRING(sink_info->active_port->name);
    }
}

PulseDeviceInfo::PulseDeviceInfo(const pa_source_info *source_info) : index(source_info->index),
                                                                      name(POINTER_TO_STRING(source_info->name)),
                                                                      channel_map(source_info->channel_map),
                                                                      cvolume(source_info->volume),
                                                                      mute(source_info->mute),
                                                                      base_volume(source_info->base_volume),
                                                                      card_index(source_info->card)
{
    for (uint32_t i = 0; i < source_info->n_ports; ++i)
    {
        auto port = std::make_shared<PulsePort>(source_info->ports[i]);
        auto iter = this->ports.emplace(port->get_name(), port);
        if (!iter.second)
        {
            LOG_WARNING("The port %s already exist.", port->get_name().c_str());
        }
    }

    if (source_info->active_port)
    {
        this->active_port_name = POINTER_TO_STRING(source_info->active_port->name);
    }
}

PulseDevice::PulseDevice(const PulseDeviceInfo &device_info) : PulseNode(device_info.index,
                                                                         device_info.name,
                                                                         device_info.channel_map,
                                                                         device_info.cvolume,
                                                                         device_info.mute,
                                                                         device_info.base_volume),
                                                               card_index_(device_info.card_index),
                                                               ports_(device_info.ports),
                                                               active_port_name_(device_info.active_port_name)
{
}

bool PulseDevice::set_active_port(const std::string &port_name)
{
    // 不支持，需要调用子类函数
    return false;
}

void PulseDevice::update(const PulseDeviceInfo &device_info)
{
    this->ports_ = device_info.ports;

    if (this->active_port_name_ != device_info.active_port_name)
    {
        this->active_port_name_ = device_info.active_port_name;
        this->active_port_changed_.emit(this->active_port_name_);
    }

    this->PulseNode::update(device_info.channel_map, device_info.cvolume, device_info.mute, device_info.base_volume);
}
}  // namespace Kiran