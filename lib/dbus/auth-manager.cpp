/*
 * @Author       : tangjie02
 * @Date         : 2020-07-24 14:43:40
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 14:37:49
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/lib/dbus/auth-manager.cpp
 */

#include "lib/dbus/auth-manager.h"

namespace Kiran
{
#define POLKIT_AUTH_CHECK_TIMEOUT 20

AuthManager *AuthManager::instance_ = nullptr;
void AuthManager::global_init()
{
    SETTINGS_PROFILE("instance: %p", instance_);
    instance_ = new AuthManager();
    instance_->init();
}

AuthManager::AuthManager() : running_auth_checks_(0)
{
}

void AuthManager::init()
{
    this->polkit_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM, POLKIT_NAME, POLKIT_PATH, POLKIT_INTERFACE);
}

void AuthManager::start_auth_check(const std::string &action, bool user_interaction, const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, AuthCheckHandler handler)
{
    SETTINGS_PROFILE("");
    std::shared_ptr<AuthCheck> auth_check = std::make_shared<AuthCheck>(invocation);
    auto timeout = Glib::MainContext::get_default()->signal_timeout();

    auth_check->cancellable = Gio::Cancellable::create();
    auth_check->cancel_connection = timeout.connect_seconds(sigc::bind(&AuthManager::cancel_auth_check, this, auth_check), POLKIT_AUTH_CHECK_TIMEOUT);
    auth_check->cancel_string = fmt::format("cancel{0}", (void *)&auth_check->cancel_connection);

    auth_check->handler = handler;

    GVariantBuilder builder1;
    GVariantBuilder builder2;

    LOG_DEBUG("action: %s user_interaction: %d sender: %s. cancel_string: %s",
              action.c_str(),
              user_interaction,
              invocation->get_sender().c_str(),
              auth_check->cancel_string.c_str());

    g_variant_builder_init(&builder1, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(&builder1, "{sv}", "name", g_variant_new_string(invocation->get_sender().c_str()));
    g_variant_builder_init(&builder2, G_VARIANT_TYPE("a{ss}"));

    auto parameters = g_variant_new("((sa{sv})sa{ss}us)", "system-bus-name", &builder1,
                                    action.c_str(), &builder2, user_interaction ? 1 : 0, auth_check->cancel_string.c_str());

    Glib::VariantContainerBase base(parameters, false);
    Gio::SlotAsyncReady res = sigc::bind(sigc::mem_fun(this, &AuthManager::finish_auth_check), auth_check);
    this->polkit_proxy_->call("CheckAuthorization", res, base);

    this->running_auth_checks_++;
}

bool AuthManager::cancel_auth_check(std::shared_ptr<AuthCheck> auth_check)
{
    SETTINGS_PROFILE("");
    auth_check->cancellable->cancel();

    Glib::VariantContainerBase base(g_variant_new("(s)", auth_check->cancel_string.c_str()), false);

    try
    {
        this->polkit_proxy_->call_sync("CancelCheckAuthorization", base);
    }
    catch (Glib::Error &e)
    {
        Gio::DBus::ErrorUtils::strip_remote_error(e);
        LOG_WARNING("Failed to cancel authorization check: %s", e.what().c_str());
    }

    // auth_check->cancel_connection.disconnect();

    return false;
}

void AuthManager::finish_auth_check(Glib::RefPtr<Gio::AsyncResult> &res, std::shared_ptr<AuthCheck> auth_check)
{
    SETTINGS_PROFILE("");
    bool authorized = true;
    bool challenge = false;
    auth_check->cancel_connection.disconnect();

    try
    {
        auto result = this->polkit_proxy_->call_finish(res);
        if (result.gobj())
        {
            g_variant_get(result.gobj(), "((bba{ss}))", &authorized, &challenge, NULL);
        }
        else
        {
            LOG_DEBUG("the result is empty.");
        }
    }
    catch (Glib::Error &e)
    {
        Gio::DBus::ErrorUtils::strip_remote_error(e);
        LOG_WARNING("Failed to check authorization: %s", e.what().c_str());
        authorized = false;
    }

    LOG_DEBUG("authorized: %d challenge: %d.", authorized, challenge);

    if (authorized)
    {
        (auth_check->handler)(auth_check->invocation);
    }
    else
    {
        auth_check->invocation->return_error(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_AUTH_FAILED, "Not authorized"));
    }

    g_return_if_fail(this->running_auth_checks_ > 0);
    this->running_auth_checks_--;
}
}  // namespace Kiran