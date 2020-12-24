///**
// * @file greeter-settings-wrapper.h
// * @brief description
// * @author yangxiaoqing <yangxiaoqing@kylinos.com.cn>
// * @copyright (c) 2020 KylinSec. All rights reserved.
//*/
//#ifndef GREETERSETTINGSWRAPPER_H
//#define GREETERSETTINGSWRAPPER_H

//#include <grp.h>
//#include <pwd.h>
//#include <shadow.h>

//#include <memory>
//#include <string>
//#include <vector>

//#include "lib/base/base.h"

//namespace Kiran
//{

//class GreeterSettingsWrapper
//{
//    // using FileChangedCallBack = void (Kiran::PasswdWrapper::*)(const Glib::RefPtr<Gio::File> &, const Glib::RefPtr<Gio::File> &, Gio::FileMonitorEvent);

//public:
//    GreeterSettingsWrapper();

//    static GreeterSettingsWrapper *get_instance() { return instance_; }

//    static void global_init();

//    static void global_deinit() { delete instance_; }

//    std::vector<PasswdShadow> get_passwds_shadows();

//    std::shared_ptr<Passwd> get_passwd_by_name(const std::string &user_name);
//    std::shared_ptr<Passwd> get_passwd_by_uid(uint64_t uid);
//    std::shared_ptr<SPwd> get_spwd_by_name(const std::string &user_name);
//    std::shared_ptr<Group> get_group_by_name(const std::string &group_name);
//    std::vector<uint32_t> get_user_groups(const std::string &user, uint32_t group);

//    sigc::signal<void, FileChangedType> &signal_file_changed()
//    {
//        return this->file_changed_;
//    };

//private:
//    void init();

//    void passwd_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);
//    void shadow_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);
//    void group_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);

//    void reload_passwd();
//    void reload_shadow();

//protected:
//    sigc::signal<void, FileChangedType> file_changed_;

//private:
//    static GreeterSettingsWrapper *instance_;

//    Glib::RefPtr<Gio::FileMonitor> passwd_monitor_;
//    Glib::RefPtr<Gio::FileMonitor> shadow_monitor_;
//    Glib::RefPtr<Gio::FileMonitor> group_monitor_;

//    std::map<std::string, std::shared_ptr<Passwd>> passwds_;
//    std::map<uint64_t, std::weak_ptr<Passwd>> passwds_by_uid_;
//    std::map<std::string, std::shared_ptr<SPwd>> spwds_;
//};
//}

//#endif // GREETERSETTINGSWRAPPER_H
