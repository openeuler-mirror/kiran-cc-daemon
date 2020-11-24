/*
 * @Author       : tangjie02
 * @Date         : 2020-11-24 09:52:15
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-24 14:24:47
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/xsettings/xsettings-xresource.h
 */
#pragma once

#include "lib/base/base.h"

namespace Kiran
{
class XSettingsXResource
{
public:
    XSettingsXResource();
    virtual ~XSettingsXResource(){};

    void init();

private:
    void on_xsettings_changed(const std::string &key);

    void update_property(std::string &props, const std::string &key, const std::string &value);
};
}  // namespace Kiran
