/*
 * @Author       : tangjie02
 * @Date         : 2020-11-20 10:08:34
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-24 20:25:59
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/xsettings/xsettings-registry.h
 */

#pragma once

#include <gdkmm.h>
// The file must be included after gdkmm.h
#include <gdk/gdkx.h>

#include <map>

#include "lib/base/base.h"

namespace Kiran
{
enum class XSettingsVarType
{
    XSETTINGS_VAR_TYPE_INT = 0,
    XSETTINGS_VAR_TYPE_STRING = 1,
    XSETTINGS_VAR_TYPE_COLOR = 2
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

class XSettingsVar
{
public:
    XSettingsVar(const std::string &name, XSettingsVarType type, uint32_t serial = 0);
    virtual ~XSettingsVar(){};

    const std::string &get_name() const { return this->name_; };
    XSettingsVarType get_type() const { return this->type_; };
    void set_serial(uint32_t serial) { this->last_change_serial_ = serial; };

public:
    virtual bool operator==(const XSettingsVar &rval) const = 0;
    virtual std::string serialize();

private:
    std::string name_;
    XSettingsVarType type_;
    uint32_t last_change_serial_;
};

class XSettingsIntVar : public XSettingsVar
{
public:
    XSettingsIntVar(const std::string &name, int32_t value, uint32_t serial = 0);
    virtual ~XSettingsIntVar(){};

public:
    virtual bool operator==(const XSettingsVar &rval) const;
    virtual bool operator==(const XSettingsIntVar &rval) const;
    virtual std::string serialize() override;

private:
    int32_t value_;
};

class XSettingsStringVar : public XSettingsVar
{
public:
    XSettingsStringVar(const std::string &name, const std::string &value, uint32_t serial = 0);
    virtual ~XSettingsStringVar(){};

public:
    virtual bool operator==(const XSettingsVar &rval) const;
    virtual bool operator==(const XSettingsStringVar &rval) const;
    virtual std::string serialize() override;

private:
    std::string value_;
};

class XSettingsColorVar : public XSettingsVar
{
public:
    XSettingsColorVar(const std::string &name, const XSettingsColor &value, uint32_t serial = 0);
    virtual ~XSettingsColorVar(){};

public:
    virtual bool operator==(const XSettingsVar &rval) const;
    virtual bool operator==(const XSettingsColorVar &rval) const;
    virtual std::string serialize() override;

private:
    XSettingsColor value_;
};

class XSettingsRegistry
{
public:
    XSettingsRegistry();
    virtual ~XSettingsRegistry();

    bool init();

    bool update(const std::string &key, int32_t value);
    bool update(const std::string &key, const Glib::ustring &value);
    bool update(const std::string &key, const XSettingsColor &value);

    void notify();

    std::shared_ptr<XSettingsVar> get_variable(const std::string &name);

private:
    bool update(const std::string &key, std::shared_ptr<XSettingsVar> var);

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

    std::map<std::string, std::shared_ptr<XSettingsVar>> variables_;
};
}  // namespace Kiran