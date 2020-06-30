/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 13:58:17
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-30 15:03:08
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-dbus.h
 */

#include <accounts_dbus_stub.h>

namespace Kiran
{
class AccountsDBus : public System::AccountsStub
{
public:
    AccountsDBus();
    virtual ~AccountsDBus();

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
};
}  // namespace Kiran