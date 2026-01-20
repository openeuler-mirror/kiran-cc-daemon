/**
 * Copyright (c) 2025 ~ 2026 KylinSec Co., Ltd.
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

#include <QObject>

class QGSettings;

namespace Kiran
{

/* 在wayland环境下，gtk程序不再通过xsettings规范获取gtksettings，而是直接从gnome的各个gsettings中获取，
   该类负责将本插件中的设置同步到gnome的gsettings中 */

class RegistryGnome : public QObject
{
    Q_OBJECT

public:
    RegistryGnome(QObject *parent = nullptr);
    virtual ~RegistryGnome() {};

    void init();

private:
    QString xftAntialias2Gnome(int antialias);
    QString xftHintStyle2Gnome(const QString &hintStyle);

private:
    void trySyncGnomeColorSchema();
    void sync2GnomeSettings(const QString &key);

private:
    const static QMap<QString, QString> s_schema2GnomeSchema;
    QGSettings *m_settings;
    QGSettings *m_gnomeDesktopSettings;
    QGSettings *m_gnomeMouseSettings;
    QGSettings *m_gnomeSoundSettings;
    QGSettings *m_gnomeXSettingsSettings;
};

}  // namespace Kiran
