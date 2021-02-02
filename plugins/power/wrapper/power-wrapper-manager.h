/**
 * @file          /kiran-cc-daemon/plugins/power/wrapper/power-wrapper-manager.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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