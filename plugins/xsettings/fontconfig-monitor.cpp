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

#include "plugins/xsettings/fontconfig-monitor.h"

namespace Kiran
{
#define FONTCONFIG_UPDATE_TIMEOUT_SECONDS 2

void FontconfigMonitor::init()
{
    FcInit();
    this->load_files_monitors();
}

void FontconfigMonitor::load_files_monitors()
{
    this->files_monitors_.clear();
    this->add_files_monitors(FcConfigGetConfigFiles(NULL));
    this->add_files_monitors(FcConfigGetFontDirs(NULL));
}

void FontconfigMonitor::add_files_monitors(FcStrList *files)
{
    const char *str;

    while ((str = (const char *)FcStrListNext(files)))
    {
        auto monitor = FileUtils::make_monitor(str, sigc::mem_fun(this, &FontconfigMonitor::file_changed));
        this->files_monitors_.push_back(monitor);
    }

    FcStrListDone(files);
}

void FontconfigMonitor::file_changed(const Glib::RefPtr<Gio::File> &file,
                                     const Glib::RefPtr<Gio::File> &other_file,
                                     Gio::FileMonitorEvent event_type)
{
    if (this->timeout_handler_)
    {
        this->timeout_handler_.disconnect();
    }
    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    this->timeout_handler_ = timeout.connect_seconds(sigc::mem_fun(this, &FontconfigMonitor::update), FONTCONFIG_UPDATE_TIMEOUT_SECONDS);
}

bool FontconfigMonitor::update()
{
    this->timeout_handler_.disconnect();
    if (!FcConfigUptoDate(NULL) && FcInitReinitialize())
    {
        this->load_files_monitors();
        this->timestamp_changed_.emit();
    }
    return false;
}
}  // namespace Kiran
