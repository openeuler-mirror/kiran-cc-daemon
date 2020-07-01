/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 13:58:17
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-30 18:00:08
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/user.h
 */

#include <act/act.h>
#include <user_dbus_stub.h>

namespace Kiran
{
class User : public System::Accounts::UserStub
{
public:
    User() = delete;
    User(ActUser *act_user);
    virtual ~User();

    void init();

protected:
    virtual void Reset(MethodInvocation &invocation);

private:
    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    uint32_t dbus_connect_id_;

    uint32_t object_register_id_;

    ActUser *act_user_;
};
}  // namespace Kiran