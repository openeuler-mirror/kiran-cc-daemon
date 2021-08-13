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
