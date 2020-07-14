/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 13:58:17
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-14 15:39:48
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/user.h
 */

#include <act/act.h>
#include <user_dbus_stub.h>

namespace Kiran
{
class User : public SystemDaemon::Accounts::UserStub
{
public:
    User() = delete;
    User(ActUser *act_user);
    virtual ~User();

    void init();

    ActUser *get_act_user() { return act_user_; }

    Glib::DBusObjectPathString get_object_path() { return this->object_path_; }

protected:
    virtual void SetUserName(const Glib::ustring &name,
                             MethodInvocation &invocation);
    virtual void SetRealName(const Glib::ustring &name,
                             MethodInvocation &invocation);
    virtual void SetEmail(const Glib::ustring &email,
                          MethodInvocation &invocation);
    virtual void SetLanguage(const Glib::ustring &language,
                             MethodInvocation &invocation);
    virtual void SetXSession(const Glib::ustring &x_session,
                             MethodInvocation &invocation);
    virtual void SetSession(const Glib::ustring &session,
                            MethodInvocation &invocation);
    virtual void SetSessionType(const Glib::ustring &session_type,
                                MethodInvocation &invocation);
    virtual void SetLocation(const Glib::ustring &location,
                             MethodInvocation &invocation);
    virtual void SetHomeDirectory(const Glib::ustring &homedir,
                                  MethodInvocation &invocation);
    virtual void SetShell(const Glib::ustring &shell,
                          MethodInvocation &invocation);
    virtual void SetIconFile(const Glib::ustring &filename,
                             MethodInvocation &invocation);
    virtual void SetLocked(bool locked,
                           MethodInvocation &invocation);
    virtual void SetAccountType(gint32 accountType,
                                MethodInvocation &invocation);
    virtual void SetPasswordMode(gint32 mode,
                                 MethodInvocation &invocation);
    virtual void SetPassword(const Glib::ustring &password,
                             const Glib::ustring &hint,
                             MethodInvocation &invocation);
    virtual void SetPasswordHint(const Glib::ustring &hint,
                                 MethodInvocation &invocation);
    virtual void SetAutomaticLogin(bool enabled,
                                   MethodInvocation &invocation);
    virtual void GetPasswordExpirationPolicy(MethodInvocation &invocation);

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool Uid_setHandler(guint64 value);
    virtual guint64 Uid_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool UserName_setHandler(const Glib::ustring &value);
    virtual Glib::ustring UserName_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool RealName_setHandler(const Glib::ustring &value);
    virtual Glib::ustring RealName_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool AccountType_setHandler(gint32 value);
    virtual gint32 AccountType_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool HomeDirectory_setHandler(const Glib::ustring &value);
    virtual Glib::ustring HomeDirectory_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool Shell_setHandler(const Glib::ustring &value);
    virtual Glib::ustring Shell_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool Email_setHandler(const Glib::ustring &value);
    virtual Glib::ustring Email_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool Language_setHandler(const Glib::ustring &value);
    virtual Glib::ustring Language_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool Session_setHandler(const Glib::ustring &value);
    virtual Glib::ustring Session_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool SessionType_setHandler(const Glib::ustring &value);
    virtual Glib::ustring SessionType_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool XSession_setHandler(const Glib::ustring &value);
    virtual Glib::ustring XSession_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool Location_setHandler(const Glib::ustring &value);
    virtual Glib::ustring Location_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool LoginFrequency_setHandler(guint64 value);
    virtual guint64 LoginFrequency_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool LoginTime_setHandler(gint64 value);
    virtual gint64 LoginTime_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool LoginHistory_setHandler(const std::vector<std::tuple<gint64, gint64, std::map<Glib::ustring, Glib::VariantBase>>> &value);
    virtual std::vector<std::tuple<gint64, gint64, std::map<Glib::ustring, Glib::VariantBase>>> LoginHistory_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool IconFile_setHandler(const Glib::ustring &value);
    virtual Glib::ustring IconFile_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool Saved_setHandler(bool value);
    virtual bool Saved_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool Locked_setHandler(bool value);
    virtual bool Locked_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool PasswordMode_setHandler(gint32 value);
    virtual gint32 PasswordMode_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool PasswordHint_setHandler(const Glib::ustring &value);
    virtual Glib::ustring PasswordHint_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool AutomaticLogin_setHandler(bool value);
    virtual bool AutomaticLogin_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool SystemAccount_setHandler(bool value);
    virtual bool SystemAccount_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool LocalAccount_setHandler(bool value);
    virtual bool LocalAccount_get();

private:
    void
    on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    uint32_t dbus_connect_id_;

    uint32_t object_register_id_;

    ActUser *act_user_;

    Glib::DBusObjectPathString object_path_;
};
}  // namespace Kiran