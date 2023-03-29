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

#include "plugins/power/wrapper/power-wrapper-manager.h"

namespace Kiran
{
PowerWrapperManager::PowerWrapperManager()
{
    this->login1_ = std::make_shared<PowerLogin1>();
    this->screensaver_ = std::make_shared<PowerScreenSaver>();
    this->session_ = std::make_shared<PowerSession>();
    this->upower_ = std::make_shared<PowerUPower>();
    this->profiles_ = std::make_shared<PowerProfiles>();
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
    KLOG_PROFILE("");

    this->login1_->init();
    this->screensaver_->init();
    this->session_->init();
    this->upower_->init();
    this->profiles_->init();
}

}  // namespace Kiran