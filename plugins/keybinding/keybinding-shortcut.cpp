/*
 * @Author       : tangjie02
 * @Date         : 2020-08-26 11:27:37
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-26 17:04:04
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/keybinding-shortcut.cpp
 */

#include "plugins/keybinding/keybinding-shortcut.h"

#include <glib/gi18n.h>

#include "lib/helper.h"

namespace Kiran
{
ShortCut::ShortCut(const std::string &kind,
                   const std::string &name,
                   const std::string &key_combination) : kind_(kind),
                                                         name_(name),
                                                         key_combination_(key_combination)
{
}

SystemShortCut::SystemShortCut(const std::string &kind,
                               const std::string &name,
                               const std::string &settings_path,
                               const std::string &settings_key,
                               const std::string &package) : ShortCut(kind, name),
                                                             settings_path_(settings_path),
                                                             settings_key_(settings_key),
                                                             package_(package)
{
    this->init();
}

bool SystemShortCut::is_valid() const
{
    RETURN_VAL_IF_FALSE(ShortCut::is_valid(), false);
    RETURN_VAL_IF_FALSE(this->settings_, false);
    return true;
}

void SystemShortCut::init()
{
    this->settings_ = Gio::Settings::create(this->settings_path_);

    if (this->settings_)
    {
        this->key_combination_ = this->settings_->get_string(this->settings_key_);
        this->settings_->signal_changed(this->settings_key_).connect(sigc::mem_fun(this, &SystemShortCut::settings_changed));
    }

    this->id_ = Glib::Checksum::compute_checksum(Glib::Checksum::CHECKSUM_MD5, this->settings_path_ + "+" + this->settings_key_);
}

void SystemShortCut::settings_changed(const Glib::ustring &key)
{
    RETURN_IF_TRUE(this->settings_key_ != key);

    auto old_keycomb = this->key_combination_;
    this->key_combination_ = this->settings_->get_string(this->settings_key_);

    if (old_keycomb != this->key_combination_)
    {
        this->keycomb_changed_.emit(old_keycomb, this->key_combination_);
    }
}
}  // namespace Kiran