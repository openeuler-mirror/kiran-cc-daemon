/*
 * @Author       : tangjie02
 * @Date         : 2020-11-28 09:54:38
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-28 10:57:52
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/src/session-guarder.h
 */
#pragma once

#include "lib/base/base.h"

namespace Kiran
{
class SessionGuarder
{
public:
    SessionGuarder(Glib::RefPtr<Glib::MainLoop> loop);
    virtual ~SessionGuarder(){};

    static SessionGuarder* get_instance() { return instance_; };

    static void global_init(Glib::RefPtr<Glib::MainLoop> loop);

    static void global_deinit() { delete instance_; };

private:
    void init();

    void on_sm_signal(const Glib::ustring& sender_name, const Glib::ustring& signal_name, const Glib::VariantContainerBase& parameters);
    void on_session_query_end();
    void on_session_end();

private:
    static SessionGuarder* instance_;

    Glib::RefPtr<Glib::MainLoop> loop_;

    // sm: session manager
    Glib::RefPtr<Gio::DBus::Proxy> sm_proxy_;
    Glib::RefPtr<Gio::DBus::Proxy> sm_private_proxy_;
};
}  // namespace Kiran
