/*
 * @Author       : tangjie02
 * @Date         : 2020-12-01 10:15:37
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-12-01 15:35:46
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/appearance/appearance-manager.h
 */
#pragma once

#include <appearance_dbus_stub.h>

#include "plugins/appearance/appearance-font.h"

namespace Kiran
{
class AppearanceManager : public SessionDaemon::AppearanceStub
{
public:
    AppearanceManager();
    virtual ~AppearanceManager(){};

    static AppearanceManager* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

protected:
    virtual void GetFont(gint32 type, MethodInvocation& invocation);
    virtual void SetFont(gint32 type, const Glib::ustring& font_name, MethodInvocation& invocation);

private:
    void init();

private:
    static AppearanceManager* instance_;

    std::shared_ptr<AppearanceFont> font_;
};
}  // namespace Kiran
