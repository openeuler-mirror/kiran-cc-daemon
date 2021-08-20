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
