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

#ifdef __cplusplus
extern "C"
{
#endif

    enum PowerDeviceType
    {
        // 计算机
        POWER_DEVICE_TYPE_COMPUTER = 0,
        /// 显示器
        POWER_DEVICE_TYPE_MONITOR,
        // 键盘
        POWER_DEVICE_TYPE_KBD,
        // 背光设备，主要是显示器和键盘
        POWER_DEVICE_TYPE_BACKLIGHT,
        POWER_DEVICE_LAST,
    };

    enum PowerSupplyMode
    {
        // 电池供电
        POWER_SUPPLY_MODE_BATTERY,
        // 交流电供电
        POWER_SUPPLY_MODE_AC,
        // UPS(交流不间断电源)供电，断电后采用蓄电池供电，UPS会将蓄电池转化为交流电
        POWER_SUPPLY_MODE_UPS,
        POWER_SUPPLY_LAST,
    };

    // 枚举类型需跟gsettings中的com.kylinsec.kiran.power-manager.*-action枚举类型保持一致，两者的数值需要对应上
    enum PowerAction
    {
        // 显示器开启
        POWER_ACTION_DISPLAY_ON = 0,
        // 显示器待机
        POWER_ACTION_DISPLAY_STANDBY,
        // 显示器挂起
        POWER_ACTION_DISPLAY_SUSPEND,
        // 显示器关闭/黑屏
        POWER_ACTION_DISPLAY_OFF,
        // 计算机挂起
        POWER_ACTION_COMPUTER_SUSPEND,
        // 计算机关机
        POWER_ACTION_COMPUTER_SHUTDOWN,
        // 计算机休眠
        POWER_ACTION_COMPUTER_HIBERNATE,
        // 不做任何操作
        POWER_ACTION_NOTHING,
        POWER_ACTION_LAST
    };

    enum PowerEvent
    {
        // 按下关机键 该参数将在后续的版本中废弃
        POWER_EVENT_PRESSED_POWEROFF = 0,
        // 释放关机键
        POWER_EVENT_RELEASE_POWEROFF = 0,
        // 按下睡眠键
        POWER_EVENT_PRESSED_SLEEP,
        // 按下挂起键
        POWER_EVENT_PRESSED_SUSPEND,
        // 按下休眠键
        POWER_EVENT_PRESSED_HIBERNATE,
        // 笔记本盖子打开
        POWER_EVENT_LID_OPEN,
        // 笔记本盖子合上
        POWER_EVENT_LID_CLOSED,
        // 按下增加显示器显示亮度键
        POWER_EVENT_PRESSED_BRIGHT_UP,
        // 按下降低显示器显示亮度键
        POWER_EVENT_PRESSED_BRIGHT_DOWN,
        // 按下增加键盘显示亮度键
        POWER_EVENT_PRESSED_KBD_BRIGHT_UP,
        // 按下降低键盘显示亮度键
        POWER_EVENT_PRESSED_KBD_BRIGHT_DOWN,
        // 按下键盘亮度显示开关切换键
        POWER_EVENT_PRESSED_KBD_BRIGHT_TOGGLE,
        // 按下锁屏键
        POWER_EVENT_PRESSED_LOCK,
        // 按下电源信息显示键
        POWER_EVENT_PRESSED_BATTERY,
        // 电池电量不足时（upower的state字段变为charge-action)
        POWER_EVENT_BATTERY_CHARGE_ACTION,
    };

    // 显示托盘图标策略，枚举类型需跟gsettings中的com.kylinsec.kiran.icon-policy枚举类型保持一致
    enum PowerTrayIconPolicy
    {
        // 总是显示
        POWER_TRAY_ICON_POLICY_ALWAYS = 0,
        // 使用电池时显示
        POWER_TRAY_ICON_POLICY_PRESENT,
        // 不显示
        POWER_TRAY_ICON_POLICY_NERVER,
    };

    enum PowerMonitorBacklightPolicy
    {
        // 自动
        POWER_MONITOR_BACKLIGHT_POLICY_AUTO = 0,
        POWER_MONITOR_BACKLIGHT_POLICY_TOOL = 1,
        POWER_MONITOR_BACKLIGHT_POLICY_X11 = 2,
    };

    /* 这里兼容两个后端的原因是power-profiles-daemon更好用，支持holdprofile接口，但支持的架构比较少。
       如果要支持D2000等系统，要使用tuned作为后端。由于tuned不支持holdprofile，如果电源电量过低时自动切换到saver
       模式后，如果电量充足了，就没法再切回原来的模式了，因此两者各有优缺点。*/
    enum PowerProfilePolicy
    {
        // 使用power-profiles-daemon作为后端
        POWER_PROFILE_POLICY_HADESS = 1,
        // 使用tuned作为后端
        POWER_PROFILE_POLICY_TUNED = 2,
    };

    enum PowerProfileMode
    {
        // 节能
        POWER_PROFILE_MODE_SAVER = 0,
        // 平衡
        POWER_PROFILE_MODE_BALANCED = 1,
        // 高性能
        POWER_PROFILE_MODE_PERFORMANCE = 2,
    };

#define POWER_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.Power"
#define POWER_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Power"

#define POWER_SCHEMA_ID "com.kylinsec.kiran.power"

// 当使用电池时，空闲超过指定时间后触发的节能行为
#define POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME "computer-battery-idle-time"
#define POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION "computer-battery-idle-action"
// 当使用电源时，空闲超过指定时间后触发的节能行为
#define POWER_SCHEMA_COMPUTER_AC_IDLE_TIME "computer-ac-idle-time"
#define POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION "computer-ac-idle-action"
// 当使用电池时，空闲超过指定时间后背光设备进入的节能状态
#define POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME "backlight-battery-idle-time"
#define POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION "backlight-battery-idle-action"
// 当使用电源时，空闲超过指定时间后背光设备进入的节能状态
#define POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME "backlight-ac-idle-time"
#define POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION "backlight-ac-idle-action"
// 显示器变暗的亮度百分比
#define POWER_SCHEMA_DISPLAY_DIMMED_BRIGHTNESS "display-dimmed-brightness"
// 显示器空闲时屏幕是否变暗
#define POWER_SCHEMA_ENABLE_DISPLAY_IDLE_DIMMED "enable-display-idle-dimmed"
// 电量过低时显示器是否变暗
#define POWER_SCHEMA_ENABLE_CHARGE_LOW_DIMMED "enable-charge-low-dimmed"
// 电量过低时计算机是否进入节能模式
#define POWER_SCHEMA_ENABLE_CHARGE_LOW_SAVER "enable-charge-low-saver"
// 按下挂起键触发的节能行为
#define POWER_SCHEMA_BUTTON_SUSPEND_ACTION "button-suspend-action"
// 按下休眠键触发的节能行为
#define POWER_SCHEMA_BUTTON_HIBERNATE_ACTION "button-hibernate-action"
// 按下电源键触发的节能行为
#define POWER_SCHEMA_BUTTON_POWER_ACTION "button-power-action"
// 盖子合上后触发的节能行为
#define POWER_SCHEMA_LID_CLOSED_ACTION "lid-closed-action"
// 当使用UPS供电时，电量不足时触发的节能行为
#define POWER_SCHEMA_UPS_CRITICAL_ACTION "ups-critical-action"
// 当使用电池供电时，电量不足时触发的节能行为
#define POWER_SCHEMA_BATTERY_CRITICAL_ACTION "battery-critical-action"
// 在什么情况下需要显示托盘图标
#define POWER_SCHEMA_TRAY_ICON_POLICY "tray-icon-policy"
// 设置获取显示器亮度值的策略，'tool'是直接操作背光设备文件，'x11'是通过xrandr接口调节亮度
#define POWER_SCHEMA_MONITOR_BACKLIGHT_POLICY "monitor-backlight-policy"
// 选用的profiles后端
#define POWER_SCHEMA_PROFILE_POLICY "profile-policy"

#ifdef __cplusplus
}
#endif