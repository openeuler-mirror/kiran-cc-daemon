/**
 * Copyright (c) 2020 ~ 2026 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */

#include "registry-kde.h"
#include <KConfig>
#include <KConfigGroup>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QGSettings>
#include <QTimer>
#include "lib/base/base.h"
#include "settings-i.h"

const QMap<QString, Kiran::RegistryKde::KConfigInfo> Kiran::RegistryKde::s_schema2KdeSchema = {
    {SETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, {"kcminputrc", "Mouse", "cursorTheme"}},
    {SETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, {"kcminputrc", "Mouse", "cursorSize"}},
};

#define KGLOBALSETTINGS_NOTIFY_TYPE_CURSOR_CHANGED 5

namespace Kiran
{
RegistryKde::RegistryKde(QObject *parent) : QObject(parent),
                                            m_settings(nullptr)
{
    m_settings = new QGSettings(SETTINGS_SCHEMA_ID, "", this);
}

void RegistryKde::init()
{
    for (const auto &key : m_settings->keys())
    {
        sync2KdeSettings(key);
    }
    connect(m_settings, &QGSettings::changed, this, &RegistryKde::sync2KdeSettings);
}

void RegistryKde::sync2KdeSettings(const QString &key)
{
    auto kdeKey = s_schema2KdeSchema[key];

    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(SETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, _hash):
    {
        KConfig config(kdeKey.file, KConfig::SimpleConfig);
        KConfigGroup group = config.group(kdeKey.group);
        group.writeEntry(kdeKey.key, m_settings->get(key));
        config.sync();
        // 晚于RegistryXSettings::notify时间点执行，避免重复通知
        QTimer::singleShot(200, this, [this]() {
            notifyKGlobalSettingsChange(KGLOBALSETTINGS_NOTIFY_TYPE_CURSOR_CHANGED, 0);
        });
        break;
    }
    default:
        break;
    }
}

void RegistryKde::notifyKGlobalSettingsChange(int type, int arg)
{
    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    message.setArguments({type, arg});
    QDBusConnection::sessionBus().send(message);
}

}  // namespace Kiran