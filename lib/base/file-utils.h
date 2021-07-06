/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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