/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
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