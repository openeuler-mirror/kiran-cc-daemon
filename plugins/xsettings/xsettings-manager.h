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

#include <QDBusContext>
#include <QMap>
#include "dbus-types.h"

class XSettingsAdaptor;
class QGSettings;
class QTimer;

namespace Kiran
{
class XSettingsRegistry;
class FontconfigMonitor;
class XSettingsXResource;

class XSettingsManager : public QObject,
                         protected QDBusContext
{
    Q_OBJECT

public:
    XSettingsManager();
    virtual ~XSettingsManager();

    static XSettingsManager *getInstance() { return m_instance; };
    static void globalInit();
    static void globalDeinit() { delete m_instance; };
    int getWindowScale();

public Q_SLOTS:
    DColor GetColor(const QString &name);
    int GetInteger(const QString &name);
    QString GetString(const QString &name);
    QStringList ListPropertyNames();
    void SetColor(const QString &name, DColor value);
    void SetInteger(const QString &name, int value);
    void SetString(const QString &name, const QString &value);

Q_SIGNALS:
    void PropertiesChanged(const QStringList &names);
    void xsettingsChanged(const QString &name);

public:
    int getXftAntialias();
    int getXftHinting();
    QString getXftHintStyle();
    QString getXftRGBA();
    int getXftDPI();
    double getFontDPI();
    QString getGtkCursorThemeName();
    int getGtkCursorThemeSize();
    int getWindowScalingFactor();
    bool getWindowScalingFactorQtSync();

private:
    void init();

    void loadFromSettings();
    void settingsChanged(const QString &key, bool isNotify);
    double getOptimizeDPI();
    void scaleSettings();
    void scaleChangeWorkarounds(int32_t scale);
    void processScreenChanged();
    void enableShowDesktopIcon();
    void processFontconfigTimestampChanged();
    void processPropertiesChanged(const QStringList &properties);

private:
    static XSettingsManager *m_instance;
    XSettingsAdaptor *m_xsettingsAdaptor;

    // 当window_scaling_factor_为0时，根据屏幕信息设置缩放，否则跟window_scaling_factor_相同。
    int32_t m_windowScale;

    QGSettings *m_xsettingsSettings;
    QGSettings *m_backgroundSettings;
    XSettingsRegistry *m_registry;
    XSettingsXResource *m_xresource;

    const static QMap<QString, QString> m_schema2Registry;
    QMap<QString, QString> m_registry2Schema;

    QTimer *m_showDesktopIconTimer;

    FontconfigMonitor *m_fontconfigMonitor;
};
}  // namespace Kiran