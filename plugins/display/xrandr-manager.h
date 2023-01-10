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

#include "lib/base/base.h"
//
#include <X11/extensions/Xrandr.h>
#include <gdk/gdkx.h>

#define DISPLAY_NEW_INTERFACE
#include "display_i.h"

namespace Kiran
{
struct ScreenInfo
{
    ScreenInfo();
    // 屏幕可以设置的最小宽度和高度
    int32_t min_width;
    int32_t min_height;
    // 屏幕可以设置的最大宽度和高度
    int32_t max_width;
    int32_t max_height;
};

struct OutputInfo
{
    OutputInfo(RROutput output_id, XRROutputInfo* output_info, const std::string& output_edid);
    RROutput id;
    // X Server的时间戳，如果时间戳小于当前X时间，则说明数据已经过期
    Time timestamp;
    // 当前使用的crtc，这个crtc必须在crtcs列表中，否则会出现错误
    RRCrtc crtc;
    // output的名字，例如VGA-1, HDMI-1
    std::string name;
    // 显示设备实际的物理宽度（毫米）
    unsigned long mm_width;
    // 显示设备实际的物理高度（毫米）
    unsigned long mm_height;
    // 显示接口是否有连接了显示设备
    bool connection;
    // 显示设备的子像素顺序
    SubpixelOrder subpixel_order;
    // 可以使用的crtc列表
    std::vector<RRCrtc> crtcs;
    // 在clones列表中的output应该和当前output有同样的crtc控制，否则会出现错误
    std::vector<RROutput> clones;
    // 可以使用的mode列表
    std::vector<RRMode> modes;
    // modes中前npreferred个mode为当前output最合适的配置
    int npreferred;
    // output的EDID属性，长度为128或者256，包含有关显示器及其性能的参数，例如供应商信息/最大图像大小/颜色设置等。
    std::string edid;
};
using OutputInfoVec = std::vector<std::shared_ptr<OutputInfo>>;
using RotationTypeVec = std::vector<uint16_t>;
using ReflectTypeVec = std::vector<uint16_t>;

#define ROTATION_ALL_MASK 0xf
#define REFLECT_ALL_MASK 0x30

struct CrtcInfo
{
    CrtcInfo(RRCrtc crtc_id, XRRCrtcInfo* crtc_info);
    RRCrtc id;
    Time timestamp;
    // 在屏幕中显示的位置
    int x;
    int y;
    // 在屏幕中可显示的区域大小。当没有旋转时，width和height分别表示在屏幕中显示的宽度和高度，
    // 这两个值会因为区域的旋转而发生变化，如果旋转90度，则width和height值进行交换
    unsigned int width;
    unsigned int height;
    // 当前使用的mode
    RRMode mode;
    // 标识显示区域是否旋转和翻转
    Rotation rotation;
    // 已经连接到当前crtc的output列表
    std::vector<RROutput> outputs;
    // 当前crtc可支持的旋转和翻转集合（位标记）
    Rotation rotations;
    // 可以连接到当前crtc的output列表
    std::vector<RROutput> possible;
};
using CrtcInfoVec = std::vector<std::shared_ptr<CrtcInfo>>;

struct ModeInfo
{
    ModeInfo();
    ModeInfo(XRRModeInfo* mode_info);
    ModeInfo(std::tuple<guint32, guint32, guint32, double, std::string> mode_info);
    RRMode id;
    // 分辨率大小
    unsigned int width;
    unsigned int height;
    // 刷新率
    double refresh_rate;
    // mode的名字
    std::string name;

    operator std::tuple<guint32, guint32, guint32, double, std::string>() const
    {
        return std::make_tuple(id, width, height, refresh_rate, name);
    }

    ModeInfo& operator=(const std::tuple<guint32, guint32, guint32, double, std::string>& value)
    {
        this->id = std::get<0>(value);
        this->width = std::get<1>(value);
        this->height = std::get<2>(value);
        this->refresh_rate = std::get<3>(value);
        this->name = std::get<4>(value);
        return *this;
    }
};
using ModeInfoVec = std::vector<std::shared_ptr<ModeInfo>>;

class XrandrManager
{
public:
public:
    XrandrManager();
    virtual ~XrandrManager();

    static XrandrManager* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    // 根据id获取output/crtc/mode信息
    std::shared_ptr<OutputInfo> get_output(RROutput id);
    std::shared_ptr<OutputInfo> get_output_by_name(const std::string& name);
    std::shared_ptr<CrtcInfo> get_crtc(RRCrtc id);
    std::shared_ptr<ModeInfo> get_mode(RRMode id);
    ModeInfoVec get_modes(const std::vector<uint32_t>& ids);

    // 获取主output
    std::shared_ptr<OutputInfo> get_primary_output();

    // 获取所有的outputs
    OutputInfoVec get_outputs();
    // 获取有显示设备连接的outputs
    OutputInfoVec get_connected_outputs();

    // 获取可用的rotation列表
    RotationTypeVec get_rotations(std::shared_ptr<CrtcInfo> crtc);
    // 获取可用的reflect列表
    ReflectTypeVec get_reflects(std::shared_ptr<CrtcInfo> crtc);

    // 获取可用的mode列表
    ModeInfoVec get_modes(std::shared_ptr<OutputInfo> output);
    // 获取最佳的mode列表
    ModeInfoVec get_prefer_modes(std::shared_ptr<OutputInfo> output);

    // 根据EDID和name生成uid
    std::string gen_uid(RROutput id);
    std::string gen_uid(std::shared_ptr<OutputInfo> output_info);

    sigc::signal<void> signal_resources_changed() { return this->resources_changed_; }

private:
    void init();
    // 初始化xrandr并检查xrandr扩展是否可用
    bool init_xrandr(std::string& err);

    void load_xrandr(bool polling);
    void load_outputs();
    void load_crtcs();
    void load_mods();
    void clear_xrandr();

    // 是否进行了垂直旋转
    bool is_rotation(Rotation rotation);

    // 获取EDID
    std::string get_edid(RROutput output_id);
    std::string get_property_string(RROutput output_id, const std::string& prop_name, int32_t length);

    //
    std::string get_connector_type(RROutput output_id);

    static GdkFilterReturn window_event(GdkXEvent* gdk_event, GdkEvent* event, gpointer data);

private:
    static XrandrManager* instance_;

    GdkDisplay* display_;
    Display* xdisplay_;
    GdkScreen* screen_;
    GdkWindow* root_window_;
    Window xroot_window_;

    int32_t event_base_;
    int32_t error_base_;

    XRRScreenResources* resources_;
    std::map<RROutput, std::shared_ptr<OutputInfo>> outputs_;
    std::map<RRCrtc, std::shared_ptr<CrtcInfo>> crtcs_;
    std::map<RRMode, std::shared_ptr<ModeInfo>> modes_;
    ScreenInfo screen_info_;
    RROutput primary_;

    Atom connector_type_atom_;

    sigc::signal<void> resources_changed_;
};

}  // namespace Kiran