/*
 * @Author       : tangjie02
 * @Date         : 2020-08-27 11:06:00
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-27 18:19:26
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/custom-shortcut.cpp
 */

#include "plugins/keybinding/custom-shortcut.h"

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>

#include "lib/log.h"
#include "lib/str-util.h"
#include "plugins/keybinding/shortcut-helper.h"

namespace Kiran
{
#define CUSTOM_SHORTCUT_FILE "custom_shortcut.ini"
#define CUSTOM_KEYFILE_NAME "name"
#define CUSTOM_KEYFILE_ACTION "action"
#define CUSTOM_KEYFILE_KEYCOMB "key_combination"
#define GEN_ID_MAX_NUM 5
#define SAVE_TO_FILE_TIMEOUT_SECONDS 2

CustomShortCutManager::CustomShortCutManager() : rand_((uint32_t)time(NULL))
{
    this->conf_file_path_ = Glib::build_filename(Glib::get_user_config_dir(),
                                                 KEYBINDING_CONF_DIR,
                                                 CUSTOM_SHORTCUT_FILE);
}

CustomShortCutManager::~CustomShortCutManager()
{
    if (this->root_window_)
    {
        this->root_window_->remove_filter(&CustomShortCutManager::window_event, this);
    }
}

CustomShortCutManager *CustomShortCutManager::instance_ = nullptr;
void CustomShortCutManager::global_init()
{
    instance_ = new CustomShortCutManager();
    instance_->init();
}

std::pair<std::string, CCError> CustomShortCutManager::add(std::shared_ptr<CustomShortCut> custom_shortcut, std::string &err)
{
    SETTINGS_PROFILE("name: %s action: %s keycomb: %s.",
                     custom_shortcut->name.c_str(),
                     custom_shortcut->action.c_str(),
                     custom_shortcut->key_combination.c_str());

    if (!this->check_valid(custom_shortcut, err))
    {
        return std::make_pair(std::string(), CCError::ERROR_INVALID_PARAMETER);
    }

    auto uid = this->gen_uid();
    if (uid.length() == 0)
    {
        err = "cannot generate unique ID for custom shortcut.";
        return std::make_pair(std::string(), CCError::ERROR_INVALID_PARAMETER);
    }
    this->change_and_save(uid, custom_shortcut);

    return std::make_pair(uid, CCError::SUCCESS);
}

CCError CustomShortCutManager::modify(const std::string &uid, std::shared_ptr<CustomShortCut> custom_shortcut, std::string &err)
{
    SETTINGS_PROFILE("name: %s action: %s keycomb: %s.",
                     custom_shortcut->name.c_str(),
                     custom_shortcut->action.c_str(),
                     custom_shortcut->key_combination.c_str());

    if (!this->check_valid(custom_shortcut, err))
    {
        return CCError::ERROR_INVALID_PARAMETER;
    }

    if (!this->keyfile_.has_group(uid))
    {
        err = fmt::format("uid {0} isn't exist", uid);
        return CCError::ERROR_INVALID_PARAMETER;
    }

    this->change_and_save(uid, custom_shortcut);
    return CCError::SUCCESS;
}

CCError CustomShortCutManager::remove(const std::string &uid, std::string &err)
{
    SETTINGS_PROFILE("id: %s.", uid.c_str());

    if (!this->keyfile_.has_group(uid))
    {
        err = fmt::format("uid {0} isn't exist", uid);
        return CCError::ERROR_INVALID_PARAMETER;
    }
    this->change_and_save(uid, nullptr);
    return CCError::SUCCESS;
}

std::shared_ptr<CustomShortCut> CustomShortCutManager::get(const std::string &uid)
{
    SETTINGS_PROFILE("id: %s.", uid.c_str());
    if (!this->keyfile_.has_group(uid))
    {
        return nullptr;
    }
    auto custom_shortcut = std::make_shared<CustomShortCut>();
    custom_shortcut->name = this->keyfile_.get_value(uid, CUSTOM_KEYFILE_NAME);
    custom_shortcut->action = this->keyfile_.get_value(uid, CUSTOM_KEYFILE_ACTION);
    custom_shortcut->key_combination = this->keyfile_.get_value(uid, CUSTOM_KEYFILE_KEYCOMB);
    return custom_shortcut;
}

std::map<std::string, std::shared_ptr<CustomShortCut>> CustomShortCutManager::get()
{
    SETTINGS_PROFILE("");
    std::map<std::string, std::shared_ptr<CustomShortCut>> custom_shortcuts;
    for (const auto &group : this->keyfile_.get_groups())
    {
        auto custom_shortcut = this->get(group);
        if (custom_shortcut)
        {
            custom_shortcuts.emplace(group, custom_shortcut);
        }
    }
    return custom_shortcuts;
}

void CustomShortCutManager::init()
{
    try
    {
        this->keyfile_.load_from_file(this->conf_file_path_, Glib::KEY_FILE_KEEP_COMMENTS);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("failed to load %s: %s.", this->conf_file_path_.c_str(), e.what().c_str());
        return;
    };

    auto display = Gdk::Display::get_default();
    this->root_window_ = display->get_default_screen()->get_root_window();
    this->root_window_->add_filter(&CustomShortCutManager::window_event, this);
    auto event_mask = this->root_window_->get_events();
    this->root_window_->set_events(event_mask | Gdk::KEY_PRESS_MASK);
}

std::string CustomShortCutManager::gen_uid()
{
    for (int i = 0; i < GEN_ID_MAX_NUM; ++i)
    {
        auto rand_str = fmt::format("Custom{0}", this->rand_.get_int());
        if (!this->keyfile_.has_group(rand_str))
        {
            return rand_str;
        }
    }
    return std::string();
}

bool CustomShortCutManager::check_valid(std::shared_ptr<CustomShortCut> custom_shortcut, std::string &err)
{
    if (custom_shortcut->name.length() == 0 ||
        custom_shortcut->action.length() == 0)
    {
        err = "the name or action is null string";
        return false;
    }

    if (ShortCutHelper::get_key_state(custom_shortcut->key_combination) == INVALID_KEYSTATE)
    {
        err = fmt::format("the format of the key_combination '{0}' is invalid.", custom_shortcut->key_combination.c_str());
        return false;
    }
    return true;
}

void CustomShortCutManager::change_and_save(const std::string &uid, std::shared_ptr<CustomShortCut> custom_shortcut)
{
    SETTINGS_PROFILE("uid: %s.", uid.c_str());

    if (custom_shortcut)
    {
        this->keyfile_.set_value(uid, CUSTOM_KEYFILE_NAME, custom_shortcut->name);
        this->keyfile_.set_value(uid, CUSTOM_KEYFILE_ACTION, custom_shortcut->action);
        this->keyfile_.set_value(uid, CUSTOM_KEYFILE_KEYCOMB, custom_shortcut->key_combination);
    }
    else
    {
        this->keyfile_.remove_group(uid);
    }

    RETURN_IF_FALSE(this->save_id_);
    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    this->save_id_ = timeout.connect_seconds(sigc::mem_fun(this, &CustomShortCutManager::save_to_file), SAVE_TO_FILE_TIMEOUT_SECONDS);
}

bool CustomShortCutManager::save_to_file()
{
    this->keyfile_.save_to_file(this->conf_file_path_);
    return false;
}

GdkFilterReturn CustomShortCutManager::window_event(GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
    RETURN_VAL_IF_TRUE(event->type != GDK_KEY_PRESS, GDK_FILTER_CONTINUE);

    CustomShortCutManager *custom_shortcut_manager = (CustomShortCutManager *)data;
    g_return_val_if_fail(CustomShortCutManager::get_instance() == custom_shortcut_manager, GDK_FILTER_REMOVE);

    for (const auto &group : custom_shortcut_manager->keyfile_.get_groups())
    {
        auto name = custom_shortcut_manager->keyfile_.get_value(group, CUSTOM_KEYFILE_NAME);
        auto action = custom_shortcut_manager->keyfile_.get_value(group, CUSTOM_KEYFILE_ACTION);
        auto key_combination = custom_shortcut_manager->keyfile_.get_value(group, CUSTOM_KEYFILE_KEYCOMB);

        // auto key_state = ShortCutHelper::get_key_state(key_combination);
        // if (key_state == ShortCutHelper::get_key_state(event))
        if (false)
        {
            std::vector<std::string> argv;
            try
            {
                argv = Glib::shell_parse_argv(action);
            }
            catch (const Glib::Error &e)
            {
                LOG_WARNING("failed to parse %s: %s.", action.c_str(), e.what().c_str());
                return GDK_FILTER_CONTINUE;
            }

            try
            {
                Glib::spawn_async(std::string(), argv, Glib::SPAWN_SEARCH_PATH);
            }
            catch (const Glib::Error &e)
            {
                LOG_WARNING("failed to exec %s: %s.", action.c_str(), e.what().c_str());
            }

            return GDK_FILTER_REMOVE;
        }
    }

    return GDK_FILTER_CONTINUE;
}

}  // namespace Kiran