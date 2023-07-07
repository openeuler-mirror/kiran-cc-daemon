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

#include "lib/base/base.h"

namespace Kiran
{
class PowerProfiles
{
public:
    // 根据gsettings的设置创建不同的子类
    static std::shared_ptr<PowerProfiles> create();

    virtual void init() = 0;

    // 设置模式
    virtual bool switch_profile(int32_t profile_mode) = 0;
    /* 临时设置模式，如果调用了ReleaseProfile，则进行恢复。如果调用了switch_profile，则不再hold当前模式
       如果返回值大于0，则表示一个cookie；如果返回值等于0；则表示无法hold，只能永久生效，功能同switch_profile；
       如果小于0则表示调用失败。*/
    virtual uint32_t hold_profile(int32_t profile_mode, const std::string &reason) = 0;
    // 释放hold_profile操作。恢复到之前的模式
    virtual void release_profile(uint32_t cookie) = 0;
    // 获取当前模式
    virtual int32_t get_active_profile() = 0;

    sigc::signal<void, int32_t> &signal_active_profile_changed() { return this->active_profile_changed_; };

protected:
    sigc::signal<void, int32_t> active_profile_changed_;
};
}  // namespace Kiran
