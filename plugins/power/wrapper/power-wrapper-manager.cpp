/**
 * @file          /kiran-cc-daemon/plugins/power/wrapper/power-wrapper-manager.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/power/wrapper/power-wrapper-manager.h"

namespace Kiran
{
PowerWrapperManager::PowerWrapperManager()
{
    this->login1_ = std::make_shared<PowerLogin1>();
    this->screensaver_ = std::make_shared<PowerScreenSaver>();
    this->session_ = std::make_shared<PowerSession>();
    this->upower_ = std::make_shared<PowerUPower>();
}

PowerWrapperManager::~PowerWrapperManager()
{
}

PowerWrapperManager* PowerWrapperManager::instance_ = nullptr;
void PowerWrapperManager::global_init()
{
    instance_ = new PowerWrapperManager();
    instance_->init();
}

void PowerWrapperManager::init()
{
    SETTINGS_PROFILE("");

    this->login1_->init();
    this->screensaver_->init();
    this->session_->init();
    this->upower_->init();
}

}  // namespace Kiran