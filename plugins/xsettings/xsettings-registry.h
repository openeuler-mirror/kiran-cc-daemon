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

#include <gdkmm.h>
// The file must be included after gdkmm.h
#include <gdk/gdkx.h>

#include <map>

#include "lib/base/base.h"

namespace Kiran
{
enum class XSettingsPropType
{
    XSETTINGS_PROP_TYPE_INT = 0,
    XSETTINGS_PROP_TYPE_STRING = 1,
    XSETTINGS_PROP_TYPE_COLOR = 2
};

struct XSettingsColor
{
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t alpha;
    bool operator==(const XSettingsColor &rval) const
    {
        return (red == rval.red) && (green == rval.green) && (blue == rval.blue) && (alpha == rval.alpha);
    }
};

class XSettingsPropertyBase
{
public:
    XSettingsPropertyBase(const std::string &name, XSettingsPropType type, uint32_t serial = 0);
    virtual ~XSettingsPropertyBase(){};

    const std::string &get_name() const { return this->name_; };
    XSettingsPropType get_type() const { return this->type_; };
    void set_serial(uint32_t serial) { this->last_change_serial_ = serial; };

public:
    virtual bool operator==(const XSettingsPropertyBase &rval) const = 0;
    virtual std::string serialize();

private:
    std::string name_;
    XSettingsPropType type_;
    uint32_t last_change_serial_;
};

using XSettingsPropertyBaseVec = std::vector<std::shared_ptr<XSettingsPropertyBase>>;

class XSettingsPropertyInt : public XSettingsPropertyBase
{
public:
    XSettingsPropertyInt(const std::string &name, int32_t value, uint32_t serial = 0);
    virtual ~XSettingsPropertyInt(){};

public:
    virtual bool operator==(const XSettingsPropertyBase &rval) const;
    virtual bool operator==(const XSettingsPropertyInt &rval) const;
    virtual std::string serialize() override;
    int32_t get_value() { return this->value_; }

private:
    int32_t value_;
};

class XSettingsPropertyString : public XSettingsPropertyBase
{
public:
    XSettingsPropertyString(const std::string &name, const std::string &value, uint32_t serial = 0);
    virtual ~XSettingsPropertyString(){};

public:
    virtual bool operator==(const XSettingsPropertyBase &rval) const;
    virtual bool operator==(const XSettingsPropertyString &rval) const;
    virtual std::string serialize() override;
    const std::string &get_value() { return this->value_; }

private:
    std::string value_;
};

class XSettingsPropertyColor : public XSettingsPropertyBase
{
public:
    XSettingsPropertyColor(const std::string &name, const XSettingsColor &value, uint32_t serial = 0);
    virtual ~XSettingsPropertyColor(){};

public:
    virtual bool operator==(const XSettingsPropertyBase &rval) const;
    virtual bool operator==(const XSettingsPropertyColor &rval) const;
    virtual std::string serialize() override;
    const XSettingsColor &get_value() { return this->value_; }

private:
    XSettingsColor value_;
};

class XSettingsRegistry
{
public:
    XSettingsRegistry();
    virtual ~XSettingsRegistry();

    bool init();

    bool update(const std::string &name, int32_t value);
    bool update(const std::string &name, const Glib::ustring &value);
    bool update(const std::string &name, const XSettingsColor &value);
    bool update(std::shared_ptr<XSettingsPropertyBase> var);

    void notify();

    std::shared_ptr<XSettingsPropertyBase> get_property(const std::string &name);
    XSettingsPropertyBaseVec get_properties();

private:
    char byte_order();

private:
    Display *xdisplay_;
    int32_t screen_;

    Atom selection_atom_;
    Atom xsettings_atom_;
    Atom manager_atom_;
    // 类型不能随意修改
    int32_t serial_;
    Window xsettings_window_;

    std::map<std::string, std::shared_ptr<XSettingsPropertyBase>> properties_;
};
}  // namespace Kiran