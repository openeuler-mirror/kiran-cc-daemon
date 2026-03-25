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

#include "display-monitor.h"
#include <kscreen/output.h>
#include <QDBusConnection>
#include <QGSettings>
#include <algorithm>
#include "display-i.h"
#include "display-manager.h"
#include "lib/base/base.h"
#include "lib/base/error.h"
#include "monitoradaptor.h"

namespace Kiran
{

#define DISPLAY_SCHEMA_ID "com.kylinsec.kiran.display"
#define DISPLAY_SCHEMA_SUPPORTED_REFLECTS "supportedReflects"
#define DISPLAY_SCHEMA_SUPPORTED_ROTATIONS "supportedRotations"

DisplayMonitor::DisplayMonitor(const KScreen::OutputPtr output)
    : m_monitorAdaptor(nullptr),
      m_displaySettings(nullptr)
{
    qDBusRegisterMetaType<quint32List>();
    qDBusRegisterMetaType<quint16List>();
    qDBusRegisterMetaType<DisplayModesStu>();
    qDBusRegisterMetaType<ListDisplayModesStu>();

    m_monitorAdaptor = new MonitorAdaptor(this);
    m_output = output->clone();
    m_displaySettings = new QGSettings(DISPLAY_SCHEMA_ID, "", this);
}

DisplayMonitor::~DisplayMonitor()
{
    dbusUnregister();
}

bool DisplayMonitor::getConnected() const
{
    return m_output->isConnected();
}

uint DisplayMonitor::getCurrentMode() const
{
    return m_output->currentModeId().toInt();
}

bool DisplayMonitor::getEnabled() const
{
    return m_output->isEnabled();
}

uint DisplayMonitor::getID() const
{
    return m_output->id();
}

quint32List DisplayMonitor::getModes() const
{
    quint32List modeIDs;
    auto modes = m_output->modes();
    for (auto mode : modes)
    {
        modeIDs.append(mode->id().toUInt());
    }
    return modeIDs;
}

QString DisplayMonitor::getName() const
{
    return m_output->name();
}

int DisplayMonitor::getNPreferred() const
{
    return m_output->preferredModes().size();
}

ushort DisplayMonitor::getReflect() const
{
    auto rotation = m_output->rotation();
    return (rotation & DISPLAY_REFLECT_ALL_MASK);
}

quint16List DisplayMonitor::getReflects() const
{
    auto reflects = m_displaySettings->get(DISPLAY_SCHEMA_SUPPORTED_REFLECTS).toStringList();
    return reflectsStr2Enum(reflects);
}

ushort DisplayMonitor::getRotation() const
{
    auto rotation = m_output->rotation();
    return (rotation & DISPLAY_ROTATION_ALL_MASK);
}

quint16List DisplayMonitor::getRotations() const
{
    auto rotations = m_displaySettings->get(DISPLAY_SCHEMA_SUPPORTED_ROTATIONS).toStringList();
    return rotationsStr2Enum(rotations);
}

int DisplayMonitor::getX() const
{
    return m_output->pos().x();
}

int DisplayMonitor::getY() const
{
    return m_output->pos().y();
}

#define SEND_PROPERTY_NOTIFY(property, propertyHump)            \
    QVariantMap changedProperties;                              \
    QVariant value;                                             \
    value.setValue(get##propertyHump());                        \
    changedProperties.insert(QStringLiteral(#property), value); \
                                                                \
    QDBusMessage signalMessage = QDBusMessage::createSignal(    \
        m_objectPath,                                           \
        QStringLiteral("org.freedesktop.DBus.Properties"),      \
        QStringLiteral("PropertiesChanged"));                   \
                                                                \
    signalMessage.setArguments({                                \
        DISPLAY_MONITOR_DBUS_INTERFACE_NAME,                    \
        changedProperties,                                      \
        QStringList(),                                          \
    });                                                         \
    QDBusConnection::sessionBus().send(signalMessage);

void DisplayMonitor::setConnected(bool connected)
{
    if (m_output->isConnected() != connected)
    {
        m_output->setConnected(connected);
        SEND_PROPERTY_NOTIFY(connected, Connected);
    }
}

void DisplayMonitor::setCurrentMode(uint currentMode)
{
    m_output->setCurrentModeId(QString::number(currentMode));
    SEND_PROPERTY_NOTIFY(currentMode, CurrentMode);
}

void DisplayMonitor::setEnabled(bool enabled)
{
    if (m_output->isEnabled() != enabled)
    {
        m_output->setEnabled(enabled);
        SEND_PROPERTY_NOTIFY(enabled, Enabled);
    }
}

void DisplayMonitor::setID(uint id)
{
    if (m_output->id() != int(id))
    {
        m_output->setId(int(id));
        SEND_PROPERTY_NOTIFY(id, ID);
    }
}

void DisplayMonitor::setName(const QString &name)
{
    if (m_output->name() != name)
    {
        m_output->setName(name);
        SEND_PROPERTY_NOTIFY(name, Name);
    }
}

void DisplayMonitor::setReflect(ushort reflect)
{
    // KScreen::OutputPtr中的rotation是吧rotation和reflect合并存储的，因此这里需要做一下拼接
    if (getReflect() != reflect)
    {
        auto rotation = getRotation();
        m_output->setRotation(KScreen::Output::Rotation(rotation + reflect));
        SEND_PROPERTY_NOTIFY(reflect, Reflect)
    }
}
void DisplayMonitor::setReflects(quint16List reflects)
{
    // 不支持修改
    return;
}

void DisplayMonitor::setRotation(ushort rotation)
{
    // KScreen::OutputPtr中的rotation是吧rotation和reflect合并存储的，因此这里需要做一下拼接
    if (getRotation() != rotation)
    {
        auto reflect = getReflect();
        m_output->setRotation(KScreen::Output::Rotation(rotation + reflect));
        SEND_PROPERTY_NOTIFY(rotation, Rotation);
    }
}
void DisplayMonitor::setRotations(quint16List)
{
    // 不支持修改
    return;
}
void DisplayMonitor::setX(int x)
{
    if (m_output->pos().x() != x)
    {
        auto y = m_output->pos().y();
        m_output->setPos(QPoint(x, y));
        SEND_PROPERTY_NOTIFY(x, X);
    }
}
void DisplayMonitor::setY(int y)
{
    if (m_output->pos().y() != y)
    {
        auto x = m_output->pos().x();
        m_output->setPos(QPoint(x, y));
        SEND_PROPERTY_NOTIFY(y, Y);
    }
}

void DisplayMonitor::Enable(bool enabled)
{
    // 状态未变化直接返回
    RETURN_IF_TRUE(enabled == getEnabled())

    // 如果状态发生了变化而且是关闭最后一个开启的显示器，则禁止该操作（至少保证有一个显示器时开启的）
    if (!enabled && DisplayManager::getInstance()->getEnabledMonitors().size() <= 1)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_ONLY_ONE_ENABLED_MONITOR);
    }

    setEnabled(enabled);
}

DisplayModesStu DisplayMonitor::GetCurrentMode()
{
    auto currentMode = m_output->currentMode();
    if (currentMode && !currentMode->id().isEmpty())
    {
        DisplayModesStu modeStu;
        modeStu.index = currentMode->id().toInt();
        modeStu.h = currentMode->size().height();
        modeStu.w = currentMode->size().width();
        modeStu.refreshRate = currentMode->refreshRate();
        return modeStu;
    }
    else
    {
        DBUS_ERROR_REPLY_AND_RETVAL(DisplayModesStu(), CCErrorCode::ERROR_DISPLAY_MODE_NOT_EXIST);
    }
}

ListDisplayModesStu DisplayMonitor::ListModes()
{
    ListDisplayModesStu modeStus;
    auto modes = m_output->modes();
    for (auto &mode : modes)
    {
        if (mode && !mode->id().isEmpty())
        {
            DisplayModesStu modeStu;
            modeStu.index = mode->id().toInt();
            modeStu.h = mode->size().height();
            modeStu.w = mode->size().width();
            modeStu.refreshRate = mode->refreshRate();
            modeStus << modeStu;
        }
        else
        {
            DBUS_ERROR_REPLY_AND_RETVAL(ListDisplayModesStu(), CCErrorCode::ERROR_DISPLAY_EXIST_NULL_MODE_IN_LIST);
        }
    }
    return modeStus;
}

ListDisplayModesStu DisplayMonitor::ListPreferredModes()
{
    ListDisplayModesStu modeStus;
    auto modeNames = m_output->preferredModes();
    for (auto &modeName : modeNames)
    {
        auto mode = m_output->mode(modeName);
        if (mode && !mode->id().isEmpty())
        {
            DisplayModesStu modeStu;
            modeStu.index = mode->id().toInt();
            modeStu.h = mode->size().height();
            modeStu.w = mode->size().width();
            modeStu.refreshRate = mode->refreshRate();
            modeStus << modeStu;
        }
        else
        {
            DBUS_ERROR_REPLY_AND_RETVAL(ListDisplayModesStu(), CCErrorCode::ERROR_DISPLAY_EXIST_NULL_MODE_IN_PREFER_LIST);
        }
    }
    return modeStus;
}

void DisplayMonitor::SetMode(uint width, uint height, double refresh_rate)
{
    auto mode = matchBestMode(width, height, refresh_rate);

    if (!mode)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_NOTFOUND_MATCH_MODE_1);
    }

    setCurrentMode(mode->id().toInt());
}

void DisplayMonitor::SetModeById(uint id)
{
    if (!m_output->mode(QString::number(id)))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_NOTFOUND_MODE_BY_ID);
    }

    setCurrentMode(id);
}

void DisplayMonitor::SetModeBySize(uint width, uint height)
{
    if (width > static_cast<uint>(INT_MAX) || height > static_cast<uint>(INT_MAX))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ARGUMENT_INVALID);
    }

    auto modes = getModesBySize(width, height);

    if (modes.size() > 0)
    {
        setCurrentMode(modes[0]->id().toInt());
    }
    else
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_NOTFOUND_MATCH_MODE_2);
    }
}

void DisplayMonitor::SetPosition(int x, int y)
{
    setX(x);
    setY(y);
}
void DisplayMonitor::SetReflect(ushort reflect)
{
    auto supportedReflects = getReflects();
    if (!supportedReflects.contains(reflect))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_REFLECT_NOT_SUPPORTED);
    }

    switch (reflect)
    {
    case DisplayReflectType::DISPLAY_REFLECT_NORMAL:
    case DisplayReflectType::DISPLAY_REFLECT_X:
    case DisplayReflectType::DISPLAY_REFLECT_XY:
    case DisplayReflectType::DISPLAY_REFLECT_Y:
        setReflect(reflect);
        break;
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_UNKNOWN_REFLECT_TYPE);
    }
}

void DisplayMonitor::SetRotation(ushort rotation)
{
    auto supportedRotations = getRotations();
    if (!supportedRotations.contains(rotation))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_ROTATION_NOT_SUPPORTED);
    }

    switch (rotation)
    {
    case DisplayRotationType::DISPLAY_ROTATION_0:
    case DisplayRotationType::DISPLAY_ROTATION_90:
    case DisplayRotationType::DISPLAY_ROTATION_180:
    case DisplayRotationType::DISPLAY_ROTATION_270:
        setRotation(rotation);
        break;
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_UNKNOWN_ROTATION_TYPE);
    }
}

void DisplayMonitor::update(const KScreen::OutputPtr output)
{
    setID(output->id());
    setName(output->name());
    setConnected(output->isConnected());
    setEnabled(output->isEnabled());
    setX(output->pos().x());
    setY(output->pos().y());
    setRotation(output->rotation() & DISPLAY_ROTATION_ALL_MASK);
    setReflect(output->rotation() & DISPLAY_REFLECT_ALL_MASK);
    setCurrentMode(output->currentModeId().toInt());

    // modes和npreferred不可写，所以这里单独做判断
    bool modesChanged = false;
    bool nPreferredChanged = false;

    if (output->modes() != m_output->modes())
    {
        modesChanged = true;
    }

    if (output->preferredModes().size() != m_output->preferredModes().size())
    {
        nPreferredChanged = true;
    }

    // 因为有些属性是没有setXXX函数，所以这里做一次全量刷新，前面调用setXXX的主要目的是触发属性变化信号
    m_output = output->clone();

    if (modesChanged)
    {
        SEND_PROPERTY_NOTIFY(modes, Modes)
    }

    if (nPreferredChanged)
    {
        SEND_PROPERTY_NOTIFY(npreferred, NPreferred)
    }
}

void DisplayMonitor::dbusRegister()
{
    m_objectPath = QString("%1/%2").arg(DISPLAY_MONITOR_OBJECT_PATH).arg(m_output->id());
    auto sessionConnection = QDBusConnection::sessionBus();

    if (!sessionConnection.registerObject(m_objectPath, DISPLAY_MONITOR_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR(appearance) << "Can't register object:" << sessionConnection.lastError();
        return;
    }
}

void DisplayMonitor::dbusUnregister()
{
    auto sessionConnection = QDBusConnection::sessionBus();
    sessionConnection.unregisterObject(m_objectPath);
}

QString DisplayMonitor::getUID()
{
    return m_output->hash();
};

KScreen::ModePtr DisplayMonitor::getBestMode()
{
    auto preferedModes = m_output->preferredModes();
    RETURN_VAL_IF_TRUE(preferedModes.size() == 0, nullptr);
    return m_output->mode(preferedModes[0]);
}

QVector<KScreen::ModePtr> DisplayMonitor::getModesBySize(uint32_t width, uint32_t height)
{
    QVector<KScreen::ModePtr> result;
    auto modes = m_output->modes();

    for (auto &mode : modes)
    {
        if (mode->size() == QSize(width, height))
        {
            result.push_back(mode);
        }
    }
    return result;
}

KScreen::ModePtr DisplayMonitor::matchBestMode(uint32_t width, uint32_t height, double refreshRate)
{
    auto modes = getModesBySize(width, height);
    KScreen::ModePtr matchMode;

    for (auto &mode : modes)
    {
        if (!matchMode ||
            (fabs(matchMode->refreshRate() - refreshRate) > fabs(mode->refreshRate() - refreshRate)))
        {
            matchMode = mode;
        }
    }
    return matchMode;
}

quint16List DisplayMonitor::reflectsStr2Enum(const QStringList &reflects) const
{
    quint16List result;

    for (const auto &reflect : reflects)
    {
        switch (shash(reflect.toLatin1().constData()))
        {
        case "normal"_hash:
            result.append(DisplayReflectType::DISPLAY_REFLECT_NORMAL);
            break;
        case "x"_hash:
            result.append(DisplayReflectType::DISPLAY_REFLECT_X);
            break;
        case "y"_hash:
            result.append(DisplayReflectType::DISPLAY_REFLECT_Y);
            break;
        case "xy"_hash:
            result.append(DisplayReflectType::DISPLAY_REFLECT_XY);
            break;
        default:
            KLOG_WARNING(display) << "Unknown reflect value in gsettings:" << reflect;
            break;
        }
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    if (result.isEmpty())
    {
        KLOG_WARNING(display) << "Empty supported reflects in gsettings, fallback to default list.";
        return quint16List{DisplayReflectType::DISPLAY_REFLECT_NORMAL};
    }

    return result;
}

quint16List DisplayMonitor::rotationsStr2Enum(const QStringList &rotations) const
{
    quint16List result;

    for (const auto &rotation : rotations)
    {
        switch (shash(rotation.toLatin1().constData()))
        {
        case "0"_hash:
            result.append(DisplayRotationType::DISPLAY_ROTATION_0);
            break;
        case "90"_hash:
            result.append(DisplayRotationType::DISPLAY_ROTATION_90);
            break;
        case "180"_hash:
            result.append(DisplayRotationType::DISPLAY_ROTATION_180);
            break;
        case "270"_hash:
            result.append(DisplayRotationType::DISPLAY_ROTATION_270);
            break;
        default:
            KLOG_WARNING(display) << "Unknown rotation value in gsettings:" << rotation;
            break;
        }
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    if (result.isEmpty())
    {
        KLOG_WARNING(display) << "Empty supported rotations in gsettings, fallback to default list.";
        return quint16List{DisplayRotationType::DISPLAY_ROTATION_0};
    }

    return result;
}

}  // namespace Kiran
