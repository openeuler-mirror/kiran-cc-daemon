/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#pragma once

#include <gdkmm.h>
//
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

#include "lib/base/base.h"

namespace Kiran
{
// 重置报警器是当显示设备从空闲状态->非空闲状态时被触发。其他报警器是当显示设备的空闲时间超过设定的时间后触发。
enum XAlarmType
{
    // 重置报警器
    XALARM_TYPE_RESET = 0,
    // 显示设备空闲一段时间变暗报警器
    XALARM_TYPE_DIM,
    XALARM_TYPE_LAST
};

struct XAlarmInfo
{
    XAlarmInfo(XAlarmType t);

    XAlarmType type;
    XSyncValue timeout;
    XSyncAlarm xalarm_id;
};

using XAlarmInfoVec = std::vector<std::shared_ptr<XAlarmInfo>>;

// 通过XSync协议设置空闲报警器(不再使用)
class PowerIdleXAlarm
{
public:
    PowerIdleXAlarm();
    virtual ~PowerIdleXAlarm();

    void init();

    // 设置一个报警器，如果不存在则添加一个新的报警器，否则更新原有报警器的超时时间，当显示设备超过timeout时间未操作时会收到X发送的Alarm信号
    bool set(XAlarmType type, uint32_t timeout);

    // 删除之前设置的报警器，首先会销毁在X中记录的报警器，然后销毁报警器的XAlarmInfo对象
    bool unset(XAlarmType type);

    // 获取已经空闲的时间
    int64_t get_xidle_time();

    // 报警器触发
    sigc::signal<void, std::shared_ptr<XAlarmInfo>> &signal_alarm_triggered() { return this->alarm_triggered_; };
    // 报警器重置
    sigc::signal<void> &signal_alarm_reset() { return this->alarm_reset_; };

private:
    // 添加一个报警器，如果已经存在则返回失败
    bool add(std::shared_ptr<XAlarmInfo> xalarm);

    // 删除一个报警器
    bool remove(std::shared_ptr<XAlarmInfo> xalarm);

    // 重置所有报警器
    void reset_all_xalarm();

    // 使用XSync协议向X注册报警器
    void register_xalarm_by_xsync(std::shared_ptr<XAlarmInfo> xalarm, XSyncTestType test_type);
    // 使用XSync协议向X销毁报警器
    void unregister_xalarm_by_xsync(std::shared_ptr<XAlarmInfo> xalarm);

    // 查找报警器
    std::shared_ptr<XAlarmInfo> find_xalarm_by_type(XAlarmType type);
    std::shared_ptr<XAlarmInfo> find_xalarm_by_id(XSyncAlarm xalarm_id);

    // XSyncValue转int64_t
    int64_t xsyncvalue_to_int64(XSyncValue value);

    static GdkFilterReturn on_event_filter_cb(GdkXEvent *gdkxevent, GdkEvent *event, gpointer data);

private:
    Display *xdisplay_;

    int32_t sync_event_base_;
    int32_t sync_error_base_;

    XSyncCounter idle_counter_;
    bool added_event_filter_;

    XAlarmInfoVec xalarms_;

    sigc::signal<void, std::shared_ptr<XAlarmInfo>> alarm_triggered_;
    sigc::signal<void> alarm_reset_;
};
}  // namespace Kiran