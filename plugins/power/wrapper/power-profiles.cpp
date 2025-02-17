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

#include "power-profiles.h"
#include <QGSettings>
#include "lib/base/base.h"
#include "power-i.h"
#include "power-profiles-hadess.h"
#include "power-profiles-tuned.h"

namespace Kiran
{
QSharedPointer<PowerProfiles> PowerProfiles::create()
{
    QGSettings powerSettings(POWER_SCHEMA_ID, "");
    auto profilesPolicy = powerSettings.get(POWER_SCHEMA_PROFILE_POLICY).toString();
    switch (shash(profilesPolicy.toUtf8().data()))
    {
    case "hadess"_hash:
        return QSharedPointer<PowerProfilesHadess>::create();
    case "tuned"_hash:
        return QSharedPointer<PowerProfilesTuned>::create();
    default:
        return QSharedPointer<PowerProfilesTuned>::create();
    }
}
}  // namespace Kiran
