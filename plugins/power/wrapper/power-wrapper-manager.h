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

#include "plugins/power/wrapper/power-login1.h"
#include "plugins/power/wrapper/power-screensaver.h"
#include "plugins/power/wrapper/power-session.h"
#include "plugins/power/wrapper/power-upower.h"

namespace Kiran
{
class PowerWrapperManager
{
public:
    PowerWrapperManager();
    virtual ~PowerWrapperManager();

    static PowerWrapperManager* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    std::shared_ptr<PowerLogin1> get_default_login1() { return this->login1_; };
    std::shared_ptr<PowerScreenSaver> get_default_screensaver() { return this->screensaver_; };
    std::shared_ptr<PowerSession> get_default_session() { return this->session_; };
    std::shared_ptr<PowerUPower> get_default_upower() { return this->upower_; };

private:
    void init();

private:
    static PowerWrapperManager* instance_;

    std::shared_ptr<PowerLogin1> login1_;
    std::shared_ptr<PowerScreenSaver> screensaver_;
    std::shared_ptr<PowerSession> session_;
    std::shared_ptr<PowerUPower> upower_;
};
}  // namespace Kiran