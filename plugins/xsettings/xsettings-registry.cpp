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

#include "plugins/xsettings/xsettings-registry.h"

#include <gdk/gdkx.h>
namespace Kiran
{
#define XSETTINGS_PAD(n, m) ((n + m - 1) & (~(m - 1)))

XSettingsPropertyBase::XSettingsPropertyBase(const std::string &name,
                                             XSettingsPropType type,
                                             uint32_t serial) : name_(name),
                                                                type_(type),
                                                                last_change_serial_(serial)
{
}

std::string XSettingsPropertyBase::serialize()
{
    std::string data;
    data.push_back(char(this->type_));
    data.push_back('\0');

    // 类型不能随意修改
    uint16_t name_len = this->name_.length();
    uint16_t name_len_pad = XSETTINGS_PAD(name_len, 4) - name_len;
    data.append(std::string((char *)&name_len, 2));
    data.append(this->name_);
    data.append(std::string(name_len_pad, '\0'));
    data.append(std::string((char *)&this->last_change_serial_, 4));
    return data;
}

XSettingsPropertyInt::XSettingsPropertyInt(const std::string &name,
                                           int32_t value,
                                           uint32_t serial) : XSettingsPropertyBase(name, XSettingsPropType::XSETTINGS_PROP_TYPE_INT, serial),
                                                              value_(value)
{
}

bool XSettingsPropertyInt::operator==(const XSettingsPropertyBase &rval) const
{
    if (rval.get_type() != XSettingsPropType::XSETTINGS_PROP_TYPE_INT)
    {
        KLOG_WARNING_XSETTINGS("Unsupported.");
        return false;
    }
    return this->operator==(dynamic_cast<const XSettingsPropertyInt &>(rval));
}

bool XSettingsPropertyInt::operator==(const XSettingsPropertyInt &rval) const
{
    return (this->get_name() == rval.get_name() && this->value_ == rval.value_);
}

std::string XSettingsPropertyInt::serialize()
{
    std::string data;
    data = this->XSettingsPropertyBase::serialize();
    data.append(std::string((char *)&this->value_, 4));
    return data;
}

XSettingsPropertyString::XSettingsPropertyString(const std::string &name,
                                                 const std::string &value,
                                                 uint32_t serial) : XSettingsPropertyBase(name, XSettingsPropType::XSETTINGS_PROP_TYPE_STRING, serial),
                                                                    value_(value)
{
}

bool XSettingsPropertyString::operator==(const XSettingsPropertyBase &rval) const
{
    if (rval.get_type() != XSettingsPropType::XSETTINGS_PROP_TYPE_STRING)
    {
        KLOG_WARNING_XSETTINGS("Unsupported.");
        return false;
    }
    return this->operator==(dynamic_cast<const XSettingsPropertyString &>(rval));
}

bool XSettingsPropertyString::operator==(const XSettingsPropertyString &rval) const
{
    return (this->get_name() == rval.get_name() && this->value_ == rval.value_);
}

std::string XSettingsPropertyString::serialize()
{
    std::string data;
    data = this->XSettingsPropertyBase::serialize();
    uint32_t str_len = this->value_.length();
    uint32_t str_len_pad = XSETTINGS_PAD(str_len, 4) - str_len;
    data.append(std::string((char *)&str_len, 4));
    data.append(this->value_);
    data.append(std::string(str_len_pad, '\0'));
    return data;
}

XSettingsPropertyColor::XSettingsPropertyColor(const std::string &name,
                                               const XSettingsColor &value,
                                               uint32_t serial) : XSettingsPropertyBase(name, XSettingsPropType::XSETTINGS_PROP_TYPE_COLOR, serial),
                                                                  value_(value)
{
}

bool XSettingsPropertyColor::operator==(const XSettingsPropertyBase &rval) const
{
    if (rval.get_type() != XSettingsPropType::XSETTINGS_PROP_TYPE_COLOR)
    {
        KLOG_WARNING_XSETTINGS("Unsupported.");
        return false;
    }
    return this->operator==(dynamic_cast<const XSettingsPropertyColor &>(rval));
}

bool XSettingsPropertyColor::operator==(const XSettingsPropertyColor &rval) const
{
    return (this->get_name() == rval.get_name() && this->value_ == rval.value_);
}

std::string XSettingsPropertyColor::serialize()
{
    std::string data;
    data = this->XSettingsPropertyBase::serialize();
    data.append(std::string((char *)&this->value_.red, 2));
    data.append(std::string((char *)&this->value_.green, 2));
    data.append(std::string((char *)&this->value_.blue, 2));
    data.append(std::string((char *)&this->value_.alpha, 2));
    return data;
}

XSettingsRegistry::XSettingsRegistry() : xdisplay_(gdk_x11_display_get_xdisplay(gdk_display_get_default())),
                                         screen_(gdk_x11_screen_get_screen_number(gdk_screen_get_default())),
                                         serial_(0)
{
    auto name = fmt::format("_XSETTINGS_S{0}", this->screen_);
    this->selection_atom_ = XInternAtom(this->xdisplay_, name.c_str(), False);
    this->xsettings_atom_ = XInternAtom(this->xdisplay_, "_XSETTINGS_SETTINGS", False);
    this->manager_atom_ = XInternAtom(this->xdisplay_, "MANAGER", False);
}

XSettingsRegistry::~XSettingsRegistry()
{
    if (this->xsettings_window_)
    {
        XDestroyWindow(this->xdisplay_, this->xsettings_window_);
    }
}

bool XSettingsRegistry::init()
{
    // 检查是否有其他xsettings插件已经在运行
    if (XGetSelectionOwner(this->xdisplay_, this->selection_atom_) != None)
    {
        KLOG_WARNING_XSETTINGS("You can only run one xsettings manager at a time.");
        return false;
    }

    this->xsettings_window_ = XCreateSimpleWindow(this->xdisplay_,
                                                  RootWindow(this->xdisplay_, this->screen_),
                                                  0, 0, 10, 10, 0,
                                                  WhitePixel(this->xdisplay_, this->screen_),
                                                  WhitePixel(this->xdisplay_, this->screen_));

    XSelectInput(this->xdisplay_, this->xsettings_window_, PropertyChangeMask);
    g_autoptr(GdkWindow) xsettings_gdk_window = gdk_x11_window_foreign_new_for_display(gdk_display_get_default(),
                                                                                       this->xsettings_window_);

    auto timestamp = gdk_x11_get_server_time(xsettings_gdk_window);

    XSetSelectionOwner(this->xdisplay_, this->selection_atom_, this->xsettings_window_, timestamp);
    // 判断设置Owner是否成功
    if (XGetSelectionOwner(this->xdisplay_, this->selection_atom_) != this->xsettings_window_)
    {
        return false;
    }

    // 其他客户端通过监听"MANAGER"事件来感知Xsettings Owner是否创建
    XClientMessageEvent xev;

    xev.type = ClientMessage;
    xev.window = RootWindow(this->xdisplay_, this->screen_);
    xev.message_type = this->manager_atom_;
    xev.format = 32;
    xev.data.l[0] = timestamp;
    xev.data.l[1] = this->selection_atom_;
    xev.data.l[2] = this->xsettings_window_;
    xev.data.l[3] = 0; /* manager specific data */
    xev.data.l[4] = 0; /* manager specific data */

    XSendEvent(this->xdisplay_,
               RootWindow(this->xdisplay_, this->screen_),
               False,
               StructureNotifyMask, (XEvent *)&xev);

    return true;
}

bool XSettingsRegistry::update(const std::string &name, int32_t value)
{
    auto var = std::make_shared<XSettingsPropertyInt>(name, value, this->serial_);
    return this->update(var);
}

bool XSettingsRegistry::update(const std::string &name, const Glib::ustring &value)
{
    auto var = std::make_shared<XSettingsPropertyString>(name, value.raw(), this->serial_);
    return this->update(var);
}

bool XSettingsRegistry::update(const std::string &name, const XSettingsColor &value)
{
    auto var = std::make_shared<XSettingsPropertyColor>(name, value, this->serial_);
    return this->update(var);
}

bool XSettingsRegistry::update(std::shared_ptr<XSettingsPropertyBase> var)
{
    RETURN_VAL_IF_TRUE(var == nullptr, true);
    auto old_var = this->get_property(var->get_name());
    if (old_var != nullptr && *old_var == *var)
    {
        return true;
    }

    this->changed_properties_.push_back(var->get_name());
    this->properties_.erase(var->get_name());
    auto iter = this->properties_.emplace(var->get_name(), var);

    // 空闲时修改，因为update可能会被连续调用多次。
    if (!this->notify_handler_)
    {
        this->notify_handler_ = Glib::signal_idle().connect(sigc::mem_fun(this, &XSettingsRegistry::notify));
    }
    return iter.second;
}

std::shared_ptr<XSettingsPropertyBase> XSettingsRegistry::get_property(const std::string &name)
{
    auto iter = this->properties_.find(name);
    if (iter == this->properties_.end())
    {
        return nullptr;
    }
    return iter->second;
}

XSettingsPropertyBaseVec XSettingsRegistry::get_properties()
{
    XSettingsPropertyBaseVec retval;
    for (auto &iter : this->properties_)
    {
        retval.push_back(iter.second);
    }
    return retval;
}

bool XSettingsRegistry::notify()
{
    KLOG_DEBUG_XSETTINGS("Notify changed properties to other client.");

    std::string data;

    // 注意：填充的相关变量类型不能随意修改

    // 填充head：byte-order + pad + SERIAL + N_SETTINGS
    int32_t nsettings = this->properties_.size();
    data.push_back(this->byte_order());
    data.append(std::string("\0\0\0", 3));
    data.append(std::string((char *)&this->serial_, 4));
    data.append(std::string((char *)&nsettings, 4));
    ++this->serial_;

    // 填充body
    for (const auto &iter : this->properties_)
    {
        data.append(iter.second->serialize());
    }

    XChangeProperty(this->xdisplay_,
                    this->xsettings_window_,
                    this->xsettings_atom_,
                    this->xsettings_atom_,
                    8,
                    PropModeReplace,
                    const_cast<unsigned char *>(reinterpret_cast<const unsigned char *>(data.c_str())),
                    data.length());

    auto changed_properties = std::move(this->changed_properties_);
    this->properties_changed_.emit(changed_properties);

    return false;
}

char XSettingsRegistry::byte_order()
{
    uint32_t myint = 0x01020304;
    return (*reinterpret_cast<char *>(&myint) == 1) ? MSBFirst : LSBFirst;
}

}  // namespace Kiran
