/**
 * @file          /kiran-cc-daemon/include/power_i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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
        // 按下关机键
        POWER_EVENT_PRESSED_POWEROFF = 0,
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

#define POWER_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.Power"
#define POWER_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Power"

#define POWER_SCHEMA_ID "com.kylinsec.kiran.power"

#define POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME "computer-battery-idle-time"
#define POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION "computer-battery-idle-action"
#define POWER_SCHEMA_COMPUTER_AC_IDLE_TIME "computer-ac-idle-time"
#define POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION "computer-ac-idle-action"
#define POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME "backlight-battery-idle-time"
#define POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION "backlight-battery-idle-action"
#define POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME "backlight-ac-idle-time"
#define POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION "backlight-ac-idle-action"
#define POWER_SCHEMA_DISPLAY_IDLE_DIM_SCALE "display-idle-dim-scale"
#define POWER_SCHEMA_BUTTON_SUSPEND_ACTION "button-suspend-action"
#define POWER_SCHEMA_BUTTON_HIBERNATE_ACTION "button-hibernate-action"
#define POWER_SCHEMA_BUTTON_POWER_ACTION "button-power-action"
#define POWER_SCHEMA_LID_CLOSED_ACTION "lid-closed-action"
#define POWER_SCHEMA_UPS_CRITICAL_ACTION "ups-critical-action"
#define POWER_SCHEMA_BATTERY_CRITICAL_ACTION "battery-critical-action"
#define POWER_SCHEMA_TRAY_ICON_POLICY "tray-icon-policy"

#ifdef __cplusplus
}
#endif