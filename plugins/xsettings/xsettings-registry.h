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

#include <xcb/xproto.h>
#include <QMap>
#include <QObject>
#include <QSharedPointer>

class QTimer;

namespace Kiran
{
class XcbConnection;

enum class XSettingsPropType
{
    XSETTINGS_PROP_TYPE_INT = 0,
    XSETTINGS_PROP_TYPE_STRING = 1,
    XSETTINGS_PROP_TYPE_COLOR = 2
};

struct XSettingsColor
{
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t alpha;
    bool operator==(const XSettingsColor &rval) const
    {
        return (red == rval.red) && (green == rval.green) && (blue == rval.blue) && (alpha == rval.alpha);
    }
};

class XSettingsPropertyBase
{
public:
    XSettingsPropertyBase(const QString &name, XSettingsPropType type, uint32_t serial = 0);
    virtual ~XSettingsPropertyBase(){};

    const QString &getName() const { return m_name; };
    XSettingsPropType getType() const { return m_type; };
    void setSerial(uint32_t serial) { m_lastChangeSerial = serial; };

public:
    virtual bool operator==(const XSettingsPropertyBase &rval) const = 0;
    virtual QByteArray serialize();

private:
    QString m_name;
    XSettingsPropType m_type;
    uint32_t m_lastChangeSerial;
};

using XSettingsPropertyBaseList = QList<QSharedPointer<XSettingsPropertyBase>>;

class XSettingsPropertyInt : public XSettingsPropertyBase
{
public:
    XSettingsPropertyInt(const QString &name, int32_t value, uint32_t serial = 0);
    virtual ~XSettingsPropertyInt(){};

public:
    virtual bool operator==(const XSettingsPropertyBase &rval) const;
    virtual bool operator==(const XSettingsPropertyInt &rval) const;
    virtual QByteArray serialize() override;
    int32_t getValue() { return m_value; }

private:
    int32_t m_value;
};

class XSettingsPropertyString : public XSettingsPropertyBase
{
public:
    XSettingsPropertyString(const QString &name, const QString &value, uint32_t serial = 0);
    virtual ~XSettingsPropertyString(){};

public:
    virtual bool operator==(const XSettingsPropertyBase &rval) const;
    virtual bool operator==(const XSettingsPropertyString &rval) const;
    virtual QByteArray serialize() override;
    const QString &getValue() { return m_value; }

private:
    QString m_value;
};

class XSettingsPropertyColor : public XSettingsPropertyBase
{
public:
    XSettingsPropertyColor(const QString &name, const XSettingsColor &value, uint32_t serial = 0);
    virtual ~XSettingsPropertyColor(){};

public:
    virtual bool operator==(const XSettingsPropertyBase &rval) const;
    virtual bool operator==(const XSettingsPropertyColor &rval) const;
    virtual QByteArray serialize() override;
    const XSettingsColor &getValue() { return m_value; }

private:
    XSettingsColor m_value;
};

class XSettingsRegistry : public QObject
{
    Q_OBJECT

public:
    XSettingsRegistry(QObject *parent = nullptr);
    virtual ~XSettingsRegistry();

    bool init();

    bool update(const QString &name, int32_t value);
    bool update(const QString &name, const QString &value);
    bool update(const QString &name, const XSettingsColor &value);
    bool update(QSharedPointer<XSettingsPropertyBase> var);

    QSharedPointer<XSettingsPropertyBase> getProperty(const QString &name) { return m_properties.value(name); };
    XSettingsPropertyBaseList getProperties() { return m_properties.values(); };

Q_SIGNALS:
    void propertiesChanged(const QStringList &names);

private:
    void notify();
    char byteOrder();

private:
    QSharedPointer<XcbConnection> m_xcbConnection;
    xcb_atom_t m_selectionAtom;
    xcb_atom_t m_xsettingsAtom;
    xcb_atom_t m_managerAtom;
    // 类型不能随意修改
    int32_t m_serial;
    xcb_window_t m_xsettingsWindow;

    QMap<QString, QSharedPointer<XSettingsPropertyBase>> m_properties;
    // 记录发生变化的属性，在改变后发送信号
    QStringList m_changedProperties;
    QTimer *m_timer;
};
}  // namespace Kiran