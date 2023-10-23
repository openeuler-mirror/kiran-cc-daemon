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

#include "lib/base/file-utils.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace Kiran
{
Glib::RefPtr<Gio::FileMonitor> FileUtils::make_monitor(const std::string &path,
                                                       const FileMonitorCallBack &callback,
                                                       Gio::FileMonitorFlags flags)
{
    auto file = Gio::File::create_for_path(path);
    try
    {
        auto monitor = file->monitor(flags);
        monitor->signal_changed().connect(callback);
        return monitor;
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Unable to monitor %s: %s", path.c_str(), e.what().c_str());
    }

    return Glib::RefPtr<Gio::FileMonitor>();
}

Glib::RefPtr<Gio::FileMonitor> FileUtils::make_monitor_file(const std::string &path,
                                                            const FileMonitorCallBack &callback,
                                                            Gio::FileMonitorFlags flags)
{
    auto file = Gio::File::create_for_path(path);
    try
    {
        auto monitor = file->monitor_file(flags);
        monitor->signal_changed().connect(callback);
        return monitor;
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Unable to file monitor %s: %s", path.c_str(), e.what().c_str());
    }

    return Glib::RefPtr<Gio::FileMonitor>();
}

Glib::RefPtr<Gio::FileMonitor> FileUtils::make_monitor_directory(const std::string &path,
                                                                 const FileMonitorCallBack &callback,
                                                                 Gio::FileMonitorFlags flags)
{
    auto file = Gio::File::create_for_path(path);
    try
    {
        auto monitor = file->monitor_directory(flags);
        monitor->signal_changed().connect(callback);
        return monitor;
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Unable to directory monitor %s: %s", path.c_str(), e.what().c_str());
    }

    return Glib::RefPtr<Gio::FileMonitor>();
}

bool FileUtils::write_contents(const std::string &path, const std::string &contents)
{
    int fp = -1;

    SCOPE_EXIT({
        if (fp > 0)
        {
            close(fp);
        }
    });

    fp = open(path.c_str(), O_WRONLY);

    if (fp < 0)
    {
        KLOG_WARNING("Failed to open file %s: %s.", path.c_str(), strerror(errno));
        return false;
    }

    if (write(fp, contents.c_str(), contents.length()) < 0)
    {
        KLOG_WARNING("Failed to write file %s: %s.", path.c_str(), strerror(errno));
        return false;
    }

    return true;
}
}  // namespace Kiran
