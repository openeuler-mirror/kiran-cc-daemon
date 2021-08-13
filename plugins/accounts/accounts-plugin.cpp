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
    KLOG_PROFILE("active accounts plugin.");

    AccountsWrapper::global_init();
    AccountsManager::global_init(AccountsWrapper::get_instance());
}

void AccountsPlugin::deactivate()
{
    KLOG_PROFILE("deactive accounts plugin.");

    AccountsManager::global_deinit();
    AccountsWrapper::global_deinit();
}

}  // namespace Kiran