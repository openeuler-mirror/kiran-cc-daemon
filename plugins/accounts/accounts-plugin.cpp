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

#include "plugins/accounts/accounts-plugin.h"

#include <cstdio>

#include <gtk3-log-i.h>
#include "plugins/accounts/accounts-manager.h"
#include "plugins/accounts/accounts-wrapper.h"

PLUGIN_EXPORT_FUNC_DEF(AccountsPlugin);

namespace Kiran
{
AccountsPlugin::AccountsPlugin()
{
}

AccountsPlugin::~AccountsPlugin()
{
}

void AccountsPlugin::activate()
{
    KLOG_DEBUG_ACCOUNTS("Active accounts plugin.");

    AccountsWrapper::global_init();
    AccountsManager::global_init(AccountsWrapper::get_instance());
}

void AccountsPlugin::deactivate()
{
    KLOG_DEBUG_ACCOUNTS("Deactive accounts plugin.");

    AccountsManager::global_deinit();
    AccountsWrapper::global_deinit();
}

}  // namespace Kiran