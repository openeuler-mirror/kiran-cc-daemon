/*
 * @Author       : tangjie02
 * @Date         : 2020-09-29 09:41:27
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-26 14:00:58
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/lib/base/file-utils.h
 */
#pragma once

#include "lib/base/base.h"

namespace Kiran
{
using FileMonitorCallBack = sigc::slot<void, const Glib::RefPtr<Gio::File> &, const Glib::RefPtr<Gio::File> &, Gio::FileMonitorEvent>;
class FileUtils
{
public:
    FileUtils(){};
    virtual ~FileUtils(){};

    static Glib::RefPtr<Gio::FileMonitor> make_monitor(const std::string &path,
                                                       const FileMonitorCallBack &callback,
                                                       Gio::FileMonitorFlags flags = Gio::FILE_MONITOR_NONE);

    static Glib::RefPtr<Gio::FileMonitor> make_monitor_file(const std::string &path,
                                                            const FileMonitorCallBack &callback,
                                                            Gio::FileMonitorFlags flags = Gio::FILE_MONITOR_NONE);

    static Glib::RefPtr<Gio::FileMonitor> make_monitor_directory(const std::string &path,
                                                                 const FileMonitorCallBack &callback,
                                                                 Gio::FileMonitorFlags flags = Gio::FILE_MONITOR_NONE);
};

}  // namespace Kiran