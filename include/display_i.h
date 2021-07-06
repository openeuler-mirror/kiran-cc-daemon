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

#ifndef DISPLAY_NEW_INTERFACE
#warning This file will be deprecated. please use display-i.h file
#endif

/*
术语：
    screen: 虚拟的屏幕。
    output: 对应主机中的一个显示接口(显示器)，这个接口可以是VGA/DVI/HDMI等。
    crtc: 是用来控制output在屏幕中显示的位置、宽度、高度、旋转和翻转等操作
    mode: 用于设置分辨率和刷新率

关系：
    一个output同一时间只对应一个crtc，一个crtc同一时间只对应一个mode
    crtc控制output放在屏幕中的位置和大小，但是不能超出屏幕的大小范围
*/

#ifdef __cplusplus
extern "C"
{
#endif

#define DISPLAY_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.Display"
#define DISPLAY_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Display"
#define DISPLAY_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SessionDaemon.Display"

    // 显示模式，只有在下列情况会使用显示模式进行设置：
    // 1. 程序第一次启动
    // 2. 有连接设备删除和添加时
    // 3. 显示调用dbus接口进行切换显示模式
    enum DisplayStyle
    {
        // 所有显示器显示内容相同
        DISPLAY_STYLE_MIRRORS,
        // 按照横向扩展的方式显示
        DISPLAY_STYLE_EXTEND,
        // 自定义模式,会从配置文件中匹配合适的显示器参数
        DISPLAY_STYLE_CUSTOM,
        // 自动模式，按照CUSTOM->EXTEND->MIRRORS的顺序进行尝试，直到设置成功
        DISPLAY_STYLE_AUTO,
        DISPLAY_STYLE_LAST,
    };

    // 显示器的旋转角度
    enum DisplayRotationType
    {
        // 不旋转
        DISPLAY_ROTATION_0 = (1 << 0),
        // 90度旋转
        DISPLAY_ROTATION_90 = (1 << 1),
        // 180度旋转
        DISPLAY_ROTATION_180 = (1 << 2),
        // 270度旋转
        DISPLAY_ROTATION_270 = (1 << 3),
    };

    // 显示器的翻转方向
    enum DisplayReflectType
    {
        // 正常
        DISPLAY_REFLECT_NORMAL = 0,
        // X方向翻转
        DISPLAY_REFLECT_X = (1 << 4),
        // Y方向翻转
        DISPLAY_REFLECT_Y = (1 << 5),
        // XY方向翻转
        DISPLAY_REFLECT_XY = (1 << 4) + (1 << 5)
    };
#ifdef __cplusplus
}
#endif