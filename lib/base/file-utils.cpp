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
    KLOG_PROFILE("path: %s", path.c_str());

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
