/*
 * @Author       : tangjie02
 * @Date         : 2020-11-26 13:41:49
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-26 14:19:20
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/xsettings/fontconfig-monitor.h
 */
#pragma once

#include <fontconfig/fontconfig.h>

#include "lib/base/base.h"
namespace Kiran
{
class FontconfigMonitor
{
public:
    FontconfigMonitor(){};
    virtual ~FontconfigMonitor(){};

    void init();

    sigc::signal<void> &signal_timestamp_changed() { return this->timestamp_changed_; };

private:
    void load_files_monitors();
    void add_files_monitors(FcStrList *files);

    void file_changed(const Glib::RefPtr<Gio::File> &file,
                      const Glib::RefPtr<Gio::File> &other_file,
                      Gio::FileMonitorEvent event_type);

    bool update();

private:
    std::vector<Glib::RefPtr<Gio::FileMonitor>> files_monitors_;
    sigc::connection timeout_handler_;
    sigc::signal<void> timestamp_changed_;
};
}  // namespace Kiran
