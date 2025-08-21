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
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QGSettings>
#include "lib/base/base.h"
#include "power-i.h"
#include "power-profiles-hadess.h"
#include "power-profiles-tuned.h"

namespace Kiran
{

#define PROFILES_HADESS_DBUS_NAME "net.hadess.PowerProfiles"

QSharedPointer<PowerProfiles> PowerProfiles::create()
{
    QGSettings powerSettings(POWER_SCHEMA_ID, "");
    bool useHadness = true;
    auto profilesPolicy = powerSettings.get(POWER_SCHEMA_PROFILE_POLICY).toString();

    switch (shash(profilesPolicy.toUtf8().data()))
    {
    case "auto"_hash:
    {
// 检测系统是否存在名为net.hadess.PowerProfiles的服务
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QStringList activatableNames = QDBusConnection::systemBus().interface()->activatableServiceNames().value();
#else
        auto reply = QDBusConnection::systemBus().interface()->callWithArgumentList(QDBus::AutoDetect,
                                                                                    QLatin1String("ListActivatableNames"),
                                                                                    QVariantList());
        QStringList activatableNames = QDBusReply<QStringList>(reply).value();
#endif
        if (!activatableNames.contains(PROFILES_HADESS_DBUS_NAME))
        {
            useHadness = false;
        }
        break;
    }
    case "hadess"_hash:
        useHadness = true;
        break;
    case "tuned"_hash:
        useHadness = false;
        break;
    default:
        break;
    }

    KLOG_INFO(power) << "Use" << (useHadness ? "hadness" : "tuned") << "profiles.";

    if (useHadness)
    {
        return QSharedPointer<PowerProfilesHadess>::create();
    }
    else
    {
        return QSharedPointer<PowerProfilesTuned>::create();
    }
}
}  // namespace Kiran
