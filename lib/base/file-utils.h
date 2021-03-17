/**
 * @file          /kiran-cc-daemon/lib/base/file-utils.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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

    // Glib::file_set_contents调用了rename函数，这里使用write函数写入内容到文件避免产生文件删除事件
    static bool write_contents(const std::string &path, const std::string &contents);
};

}  // namespace Kiran