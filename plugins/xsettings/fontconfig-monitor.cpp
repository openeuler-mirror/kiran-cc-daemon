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

    while ((str = reinterpret_cast<const char *>(FcStrListNext(files))))
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
