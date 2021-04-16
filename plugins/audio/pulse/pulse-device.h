/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-device.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "plugins/audio/pulse/pulse-node.h"
#include "plugins/audio/pulse/pulse-port.h"

namespace Kiran
{
struct PulseDeviceInfo
{
public:
    PulseDeviceInfo(const pa_sink_info *sink_info);
    PulseDeviceInfo(const pa_source_info *source_info);

    uint32_t index;
    std::string name;
    // 控制声道
    pa_channel_map channel_map;
    // 声音
    pa_cvolume cvolume;
    // 静音
    int32_t mute;
    // 基本音量，一般又硬件决定
    pa_volume_t base_volume;
    // 设备对应的声卡
    uint32_t card_index;
    // 设备可用的端口
    std::map<std::string, std::shared_ptr<PulsePort>> ports;
    // 活动端口
    std::string active_port_name;
};

class PulseDevice : public PulseNode
{
public:
    PulseDevice(const PulseDeviceInfo &device_info);
    virtual ~PulseDevice(){};

    // 设置活动的端口
    virtual bool set_active_port(const std::string &port_name);

    // 获取活动的端口
    std::string get_active_port() { return this->active_port_name_; };
    // 根据名字获取端口
    std::shared_ptr<PulsePort> get_port(const std::string &port_name) { return MapHelper::get_value(this->ports_, port_name); };
    // 获取所有绑定端口
    PulsePortVec get_ports() { return MapHelper::get_values(this->ports_); };
    // 获取声卡索引
    uint32_t get_card_index() { return this->card_index_; };

    // 相关信号
    sigc::signal<void, const std::string &> &signal_active_port_changed() { return this->active_port_changed_; };

protected:
    void update(const PulseDeviceInfo &device_info);

private:
    // card index
    uint32_t card_index_;
    // sink ports
    std::map<std::string, std::shared_ptr<PulsePort>> ports_;
    // active sink port
    std::string active_port_name_;

    sigc::signal<void, const std::string &> active_port_changed_;
};
}  // namespace Kiran