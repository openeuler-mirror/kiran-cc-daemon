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