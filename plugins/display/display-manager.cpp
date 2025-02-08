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

#include "display-manager.h"
#include <kscreen/config.h>
#include <kscreen/configmonitor.h>
#include <kscreen/getconfigoperation.h>
#include <kscreen/output.h>
#include <kscreen/setconfigoperation.h>
#include <QFileInfo>
#include <QGSettings>
#include <QPluginLoader>
#include <QProcess>
#include <QSet>
#include <QSize>
#include <QStandardPaths>
#include <fstream>
#include "config.h"
#include "display-i.h"
#include "display-monitor.h"
#include "display-util.h"
#include "display.hxx"
#include "displayadaptor.h"
#include "lib/base/base.h"
#include "xsettings-i.h"

uint qHash(const QSize &size)
{
    return size.width() * size.height();
}

namespace Kiran
{
#define DISPLAY_SCHEMA_ID "com.kylinsec.kiran.display"
#define DISPLAY_SCHEMA_STYLE "displayStyle"
#define DISPLAY_SCHEMA_DYNAMIC_SCALING_WINDOW "dynamicScalingWindow"
#define DISPLAY_SCHEMA_MAX_SCREEN_RECORD_NUMBER "maxScreenRecordNumber"
#define DISPLAY_SCHEMA_SCREEN_CHANGED_ADAPT "screenChangedAdaptation"

#define DISPLAY_CONF_DIR "kylinsec/" PROJECT_NAME "/display"
#define DISPLAY_FILE_NAME "display.xml"

#define MONITOR_JOIN_CHAR ","
#define XRANDR_CMD "xrandr"

#define DEFAULT_MAX_SCREEN_RECORD_NUMBER 100

DisplayManager::DisplayManager() : m_currentConfig(nullptr),
                                   m_defaultStyle(DisplayStyle::DISPLAY_STYLE_EXTEND),
                                   m_windowScalingFactor(0),
                                   m_dynamicScalingWindow(false),
                                   m_maxScreenRecordNumber(DEFAULT_MAX_SCREEN_RECORD_NUMBER)
{
    m_displayAdaptor = new DisplayAdaptor(this);
    m_configFilePath = QString("%1/%2/%3")
                           .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                           .arg(DISPLAY_CONF_DIR)
                           .arg(DISPLAY_FILE_NAME);

    m_configMonitor = KScreen::ConfigMonitor::instance();
    m_displaySettings = new QGSettings(DISPLAY_SCHEMA_ID, "", this);
    m_xsettingsSettings = new QGSettings(XSETTINGS_SCHEMA_ID, "", this);
}

DisplayManager::~DisplayManager()
{
}

DisplayManager *DisplayManager::m_instance = nullptr;
void DisplayManager::globalInit()
{
    m_instance = new DisplayManager();
    m_instance->preInit();
}

QList<QSharedPointer<DisplayMonitor>> DisplayManager::getConnectedMonitors()
{
    QList<QSharedPointer<DisplayMonitor>> monitors;
    for (const auto &monitor : m_monitors)
    {
        if (monitor->getConnected())
        {
            monitors.push_back(monitor);
        }
    }
    return monitors;
}

QList<QSharedPointer<DisplayMonitor>> DisplayManager::getEnabledMonitors()
{
    QList<QSharedPointer<DisplayMonitor>> monitors;
    for (const auto &monitor : m_monitors)
    {
        if (monitor->getEnabled())
        {
            monitors.push_back(monitor);
        }
    }
    return monitors;
}

uint DisplayManager::getDefaultStyle() const
{
    return m_defaultStyle;
}

QString DisplayManager::getPrimary() const
{
    return m_primary;
}
int DisplayManager::getWindowScalingFactor() const
{
    return m_windowScalingFactor;
}

#define SEND_PROPERTY_NOTIFY(property, propertyHump)                          \
    QVariantMap changedProperties;                                            \
    changedProperties.insert(QStringLiteral(#property), get##propertyHump()); \
                                                                              \
    QDBusMessage signalMessage = QDBusMessage::createSignal(                  \
        DISPLAY_OBJECT_PATH,                                                  \
        QStringLiteral("org.freedesktop.DBus.Properties"),                    \
        QStringLiteral("PropertiesChanged"));                                 \
                                                                              \
    signalMessage.setArguments({                                              \
        DISPLAY_DBUS_INTERFACE_NAME,                                          \
        changedProperties,                                                    \
        QStringList(),                                                        \
    });                                                                       \
    QDBusConnection::sessionBus().send(signalMessage);

void DisplayManager::setDefaultStyle(uint style)
{
    RETURN_IF_TRUE(m_defaultStyle == DisplayStyle(style));

    m_defaultStyle = DisplayStyle(style);
    auto defaultStyleFromSettings = styleStr2Enum(m_displaySettings->get(DISPLAY_SCHEMA_STYLE).toString());
    if (defaultStyleFromSettings != int32_t(m_defaultStyle))
    {
        m_displaySettings->set(DISPLAY_SCHEMA_STYLE, styleEnum2Str(m_defaultStyle));
    }

    SEND_PROPERTY_NOTIFY(default_style, DefaultStyle)
}

void DisplayManager::setPrimary(const QString &name)
{
    m_primary = name;
    SEND_PROPERTY_NOTIFY(primary, Primary)
}

void DisplayManager::setWindowScalingFactor(int window_scaling_factor)
{
    m_windowScalingFactor = window_scaling_factor;
    SEND_PROPERTY_NOTIFY(window_scaling_factor, WindowScalingFactor)
}

void DisplayManager::ApplyChanges()
{
    CCErrorCode errorCode = CCErrorCode::SUCCESS;
    if (!apply(errorCode))
    {
        DBUS_ERROR_REPLY_AND_RET(errorCode);
    }
}

QStringList DisplayManager::ListMonitors()
{
    QStringList objectPaths;
    for (const auto &monitor : m_monitors)
    {
        objectPaths.push_back(monitor->getObjectPath());
    }
    return objectPaths;
}

void DisplayManager::RestoreChanges()
{
    CCErrorCode errorCode = CCErrorCode::SUCCESS;
    if (!switchStyle(m_defaultStyle, errorCode))
    {
        DBUS_ERROR_REPLY_AND_RET(errorCode);
    }
}

void DisplayManager::Save()
{
    CCErrorCode errorCode = CCErrorCode::SUCCESS;

    if (!saveConfig(errorCode))
    {
        DBUS_ERROR_REPLY_AND_RET(errorCode);
    }
}

void DisplayManager::SetDefaultStyle(uint style)
{
    if (style >= DisplayStyle::DISPLAY_STYLE_LAST)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_UNKNOWN_DISPLAY_STYLE_2);
    }
    setDefaultStyle(style);
}

void DisplayManager::DisplayManager::SetPrimary(const QString &name)
{
    KLOG_INFO(display) << "Set primary name to" << name;

    if (name.length() == 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_PRIMARY_MONITOR_IS_EMPTY);
    }

    if (!getMonitorByName(name))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_DISPLAY_NOTFOUND_PRIMARY_MONITOR_BY_NAME);
    }

    setPrimary(name);
}

void DisplayManager::SetWindowScalingFactor(int windowScalingFactor)
{
    RETURN_IF_TRUE(getWindowScalingFactor() == windowScalingFactor)

    if (!m_dynamicScalingWindow)
    {
        auto tipsInfo = QString("%1").arg(tr("The scaling rate can only take effect after logging out and logging in again."));
        auto result = QProcess::startDetached("/usr/bin/notify-send", QStringList() << tipsInfo);

        if (!result)
        {
            KLOG_WARNING(display) << "Failed to run notify-send";
        }
    }

    setWindowScalingFactor(windowScalingFactor);
}

void DisplayManager::SwitchStyle(uint style)
{
    CCErrorCode errorCode = CCErrorCode::SUCCESS;
    if (!switchStyle(DisplayStyle(style), errorCode))
    {
        DBUS_ERROR_REPLY_AND_RET(errorCode);
    }
}

void DisplayManager::preInit()
{
    connect(new KScreen::GetConfigOperation,
            &KScreen::GetConfigOperation::finished,
            this,
            [this](KScreen::ConfigOperation *op)
            {
                if (op->hasError())
                {
                    KLOG_WARNING(display) << "Failed to get init configuration" << op->errorString();
                    return;
                }

                this->m_currentConfig = qobject_cast<KScreen::GetConfigOperation *>(op)->config();
                this->init();
            });
}

void DisplayManager::init()
{
    loadSettings();
    loadMonitors();
    loadConfig();

    m_configMonitor->addConfig(m_currentConfig);

    connect(m_displaySettings, &QGSettings::changed, this, &DisplayManager::processSettingsChanged);
    connect(m_configMonitor, &KScreen::ConfigMonitor::configurationChanged, this, &DisplayManager::processConfigureChanged);

    CCErrorCode errorCode = CCErrorCode::SUCCESS;
    if (!switchStyleAndSave(m_defaultStyle, errorCode))
    {
        KLOG_WARNING(display) << KCD_ERROR2STR(errorCode);
    }

    /* window_scaling_factor的初始化顺序：
       1. 先读取xsettings中的window-scaling-factor属性; (load_settings)
       2. 读取monitor.xml中维护的window-scaling-factor值 （switch_style_and_save）
       3. 如果第2步和第1步的值不相同，则说明在上一次进入会话时用户修改了缩放率，需要在这一次进入会话时生效，
          因此需要将monitor.xml中的缩放率更新到xsettings中的window-scaling-factor属性中*/
    if (m_windowScalingFactor != m_xsettingsSettings->get(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR).toInt())
    {
        m_xsettingsSettings->set(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, m_windowScalingFactor);
    }

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerService(DISPLAY_DBUS_NAME))
    {
        KLOG_WARNING(appearance) << "Failed to register dbus name: " << DISPLAY_DBUS_NAME;
        return;
    }

    if (!sessionConnection.registerObject(DISPLAY_OBJECT_PATH, DISPLAY_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR(appearance) << "Can't register object:" << sessionConnection.lastError();
        return;
    }
}

void DisplayManager::loadSettings()
{
    m_defaultStyle = DisplayStyle(styleStr2Enum(m_displaySettings->get(DISPLAY_SCHEMA_STYLE).toString()));
    m_dynamicScalingWindow = m_displaySettings->get(DISPLAY_SCHEMA_DYNAMIC_SCALING_WINDOW).toBool();
    m_windowScalingFactor = m_xsettingsSettings->get(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR).toInt();
    m_maxScreenRecordNumber = m_displaySettings->get(DISPLAY_SCHEMA_MAX_SCREEN_RECORD_NUMBER).toInt();
}

void DisplayManager::loadMonitors()
{
    // 加载主显示器

    auto primaryOutput = m_currentConfig->primaryOutput();
    auto primaryName = primaryOutput ? primaryOutput->name() : QString();
    setPrimary(primaryName);

    // 删除已经不存在的monitor
    for (auto iter = m_monitors.begin(); iter != m_monitors.end();)
    {
        auto output = m_currentConfig->output(iter.key());
        if (!output || !output->isConnected())
        {
            iter.value()->dbusUnregister();
            m_monitors.erase(iter++);
        }
        else
        {
            ++iter;
        }
    }

    auto outputs = m_currentConfig->connectedOutputs();

    for (const auto &output : outputs)
    {
        auto iter = m_monitors.find(output->id());
        if (iter == m_monitors.end())
        {
            auto monitor = QSharedPointer<DisplayMonitor>::create(output);
            m_monitors.insert(output->id(), monitor);
            monitor->dbusRegister();
        }
        else
        {
            iter.value()->update(output);
        }
    }
}

void DisplayManager::loadConfig()
{
    if (QFileInfo::exists(m_configFilePath))
    {
        try
        {
            m_displayConfig = display(m_configFilePath.toStdString(), xml_schema::Flags::dont_validate);
        }
        catch (const xml_schema::Exception &e)
        {
            KLOG_WARNING(display) << "Failed to load config file" << m_configFilePath << ". Error:" << e.what();
            m_displayConfig = nullptr;
        }
    }
    else
    {
        KLOG_DEBUG(display) << m_configFilePath << "is not exist.";
    }
    return;
}

bool DisplayManager::applyConfig(CCErrorCode &errorCode)
{
    if (!m_displayConfig)
    {
        errorCode = CCErrorCode::ERROR_DISPLAY_CONFIG_IS_EMPTY;
        return false;
    }

    auto monitorsUID = getMonitorsUID();
    const auto &screens = m_displayConfig->screen();
    bool result = false;

    for (const auto &screen : screens)
    {
        const auto &monitors = screen.monitor();
        auto monitorsConfigID = getCMonitorsUID(monitors);
        if (monitorsUID == monitorsConfigID)
        {
            KLOG_INFO(display) << "Match ids and the ids is" << monitorsUID;
            if (applyScreenConfig(screen, errorCode))
            {
                result = true;
                break;
            }
        }
    }

    if (!result && errorCode == CCErrorCode::SUCCESS)
    {
        errorCode = CCErrorCode::ERROR_DISPLAY_CONFIG_ITEM_NOTFOUND;
    }
    return result;
}

bool DisplayManager::applyScreenConfig(const ScreenConfigInfo &screenConfig, CCErrorCode &errorCode)
{
    const auto &cMonitors = screenConfig.monitor();

    setPrimary(screenConfig.primary().c_str());
    setWindowScalingFactor(screenConfig.window_scaling_factor());

    for (const auto &cMonitor : cMonitors)
    {
        auto monitor = matchBestMonitor(cMonitor.uid().c_str(), cMonitor.name().c_str());

        if (!monitor)
        {
            KLOG_WARNING(display) << "Cannot find monitor, uid is" << cMonitor.uid().c_str()
                                  << ", name is" << cMonitor.name().c_str();
            return false;
        }

        /* 一般情况下uid相同时name也是相同的，但是有些特殊情况会出现不一样，这里uid主要是为了唯一标识一台显示器，
           而name是用来区分显示器接口的，比如有一台显示器最开始是接入在HDMI-1，后面改到HDMI-2了，那么在能获取到edid的
           情况下uid是不变的，但是name会发生变化。如果出现name不一样的情况下这里仅仅记录日志，方便后续跟踪问题。*/
        if (cMonitor.name() != monitor->getName().toStdString())
        {
            KLOG_INFO(display) << "The monitor name is dismatch. config name is" << cMonitor.name().c_str()
                               << ", monitor name is" << monitor->getName();
        }

        if (!cMonitor.enabled())
        {
            monitor->setEnabled(false);
            monitor->setX(0);
            monitor->setY(0);
            monitor->setRotation(uint16_t(DisplayRotationType::DISPLAY_ROTATION_0));
            monitor->setReflect(uint16_t(DisplayReflectType::DISPLAY_REFLECT_NORMAL));
            monitor->setCurrentMode(0);
        }
        else
        {
            // 只有在显示器开启状态下才能取匹配mode，因为显示器关闭状态下c_monitor里面保持的分辨率都是0x0
            auto mode = monitor->matchBestMode(cMonitor.width(), cMonitor.height(), cMonitor.refresh_rate());
            if (!mode)
            {
                KLOG_WARNING(display) << "Cannot match the mode. width:" << cMonitor.width()
                                      << ", height:" << cMonitor.height()
                                      << ", refresh:" << cMonitor.refresh_rate();
                return false;
            }

            monitor->setEnabled(true);
            monitor->setX(cMonitor.x());
            monitor->setY(cMonitor.y());
            monitor->setRotation(uint16_t(DisplayUtil::rotationStr2Enum(cMonitor.rotation().c_str())));
            monitor->setReflect(uint16_t(DisplayUtil::reflectStr2Enum(cMonitor.reflect().c_str())));
            monitor->setCurrentMode(mode->id().toInt());
        }
    }

    return true;
}

void DisplayManager::fillScreenConfig(ScreenConfigInfo &screenConfig)
{
    screenConfig.timestamp((uint32_t)time(NULL));
    screenConfig.primary(m_primary.toStdString());
    screenConfig.window_scaling_factor(m_windowScalingFactor);

    for (auto &monitor : m_monitors)
    {
        auto output = m_currentConfig->output(monitor->getID());
        auto mode = output->currentMode();

        if (!mode || !monitor->getEnabled())
        {
            MonitorConfigInfo cMonitor(monitor->getUID().toStdString(),
                                       monitor->getName().toStdString(),
                                       false,
                                       0,
                                       0,
                                       0,
                                       0,
                                       DisplayUtil::rotationEnum2str(DisplayRotationType::DISPLAY_ROTATION_0).toStdString(),
                                       DisplayUtil::reflectEnum2str(DisplayReflectType::DISPLAY_REFLECT_NORMAL).toStdString(),
                                       0.0);

            screenConfig.monitor().push_back(std::move(cMonitor));
        }
        else
        {
            MonitorConfigInfo cMonitor(monitor->getUID().toStdString(),
                                       monitor->getName().toStdString(),
                                       true,
                                       monitor->getX(),
                                       monitor->getY(),
                                       mode->size().width(),
                                       mode->size().height(),
                                       DisplayUtil::rotationEnum2str(DisplayRotationType(monitor->getRotation())).toStdString(),
                                       DisplayUtil::reflectEnum2str(DisplayReflectType(monitor->getReflect())).toStdString(),
                                       mode->refreshRate());
            screenConfig.monitor().push_back(std::move(cMonitor));
        }
    }
}

bool DisplayManager::saveConfig(CCErrorCode &errorCode)
{
    if (!m_displayConfig)
    {
        m_displayConfig = std::unique_ptr<DisplayConfigInfo>(new DisplayConfigInfo());
    }

    // 禁止保存没有开启任何显示器的配置，这可能会导致下次进入会话屏幕无法显示
    if (getEnabledMonitors().size() == 0)
    {
        KLOG_WARNING(display) << "It is forbidden to save the configuration without any display turned on, "
                                 "which may cause the next session screen not to be displayed.";
        errorCode = CCErrorCode::ERROR_DISPLAY_NO_ENABLED_MONITOR;
        return false;
    }

    auto monitorsUID = getMonitorsUID();
    auto &cScreens = m_displayConfig->screen();
    bool matched = false;
    ScreenConfigInfo usedConfig(0, "", 0);

    fillScreenConfig(usedConfig);
    for (auto &cScreen : cScreens)
    {
        auto &cMonitors = cScreen.monitor();
        auto c_monitors_uid = getCMonitorsUID(cMonitors);
        if (monitorsUID == c_monitors_uid)
        {
            cScreen = usedConfig;
            matched = true;
            break;
        }
    }

    if (!matched)
    {
        m_displayConfig->screen().push_back(usedConfig);
    }

    if (cScreens.size() > m_maxScreenRecordNumber)
    {
        auto oldest_screen = cScreens.begin();
        for (auto iter = cScreens.begin(); iter != cScreens.end(); iter++)
        {
            if ((*iter).timestamp() < (*oldest_screen).timestamp())
            {
                oldest_screen = iter;
            }
        }

        if (oldest_screen != cScreens.end())
        {
            cScreens.erase(oldest_screen);
        }
    }

    RETURN_VAL_IF_FALSE(saveToFile(errorCode), false);

    return true;
}

bool DisplayManager::apply(CCErrorCode &errorCode)
{
    // 如果使用的是nvidia驱动，当没有接入任何显示器时,会将output的分辨率设置为8x8，导致底部面板不可见且后面无法恢复。
    if (getEnabledMonitors().size() == 0)
    {
        KLOG_WARNING(display) << "Cannot find enabled monitor.";
        errorCode = CCErrorCode::ERROR_DISPLAY_NO_ENABLED_MONITOR;
        return false;
    }

    if (m_dynamicScalingWindow)
    {
        // 应用缩放因子
        m_xsettingsSettings->set(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, m_windowScalingFactor);
    }

    QSharedPointer<DisplayMonitor> primaryMonitor;

    // 如果没有设置主显示器，这里会默认使用第一个遍历到的显示器作为主显示器，因为必须要在已开启的显示器中设置一个主显示器，否则可能出现鼠标键盘操作卡顿情况
    for (const auto &monitor : m_monitors)
    {
        if (!monitor->getEnabled())
        {
            continue;
        }

        if (!primaryMonitor || monitor->getName() == m_primary)
        {
            primaryMonitor = monitor;
        }
    }

    KScreen::OutputList outputs;
    for (const auto &monitor : m_monitors)
    {
        auto output = monitor->getOutput();
        outputs.insert(output->id(), output);
    }
    m_currentConfig->setOutputs(outputs);

    auto setop = new KScreen::SetConfigOperation(m_currentConfig);
    if (!setop->exec())
    {
        errorCode = CCErrorCode::ERROR_DISPLAY_APPLY_FAILED;
        KLOG_WARNING(display) << "Failed to apply display config which error is" << setop->errorString();
        return false;
    }

    return true;
}

bool DisplayManager::switchStyle(DisplayStyle style, CCErrorCode &errorCode)
{
    KLOG_INFO(display) << "Switch style to" << styleEnum2Str(style);

    switch (style)
    {
    case DisplayStyle::DISPLAY_STYLE_MIRRORS:
        RETURN_VAL_IF_FALSE(switchToMirrors(errorCode), false);
        break;
    case DisplayStyle::DISPLAY_STYLE_EXTEND:
        switchToExtend();
        break;
    case DisplayStyle::DISPLAY_STYLE_CUSTOM:
        RETURN_VAL_IF_FALSE(switchToCustom(errorCode), false);
        break;
    case DisplayStyle::DISPLAY_STYLE_AUTO:
        switchToAuto();
        break;
    default:
        errorCode = CCErrorCode::ERROR_DISPLAY_UNKNOWN_DISPLAY_STYLE_1;
        return false;
    }

    // 因为自定义模式下可能由于参数错误导致设置失败，为了增强鲁棒性，这里再做一次扩展模式的设置尝试
    if (!apply(errorCode))
    {
        KLOG_WARNING(display) << "The first apply failed:" << KCD_ERROR2STR(errorCode) << ",try use extend mode ";
        switchToExtend();
        errorCode = CCErrorCode::SUCCESS;
        if (!apply(errorCode))
        {
            KLOG_WARNING(display) << "The second apply also failed:" << KCD_ERROR2STR(errorCode);
            return false;
        }
    }
    return true;
}

bool DisplayManager::switchStyleAndSave(DisplayStyle style, CCErrorCode &error_code)
{
    RETURN_VAL_IF_FALSE(switchStyle(style, error_code), false);
    RETURN_VAL_IF_FALSE(saveConfig(error_code), false);
    return true;
}

bool DisplayManager::switchToMirrors(CCErrorCode &errorCode)
{
    auto connectedMonitors = getConnectedMonitors();

    QSet<QSize> sameSizes;
    const QSize maxScreenSize = m_currentConfig->screen()->maxSize();

    // 计算所有显示器拥有的相同分辨率放入sameSizes
    for (const auto &monitor : connectedMonitors)
    {
        auto output = monitor->getOutput();
        QSet<QSize> modeSizes;
        for (const auto &mode : output->modes())
        {
            const QSize size = mode->size();
            if (size.width() > maxScreenSize.width() || size.height() > maxScreenSize.height())
            {
                continue;
            }
            modeSizes.insert(mode->size());
        }

        if (sameSizes.size() == 0)
        {
            sameSizes = modeSizes;
        }
        else
        {
            sameSizes.intersect(modeSizes);
        }

        if (sameSizes.size() == 0)
        {
            break;
        }
    }

    KLOG_DEBUG(display) << "Same sizes: " << sameSizes;

    if (sameSizes.size() == 0)
    {
        errorCode = CCErrorCode::ERROR_DISPLAY_COMMON_MODE_NOTFOUND;
        return false;
    }

    // 如果有多个相同的，取面积最大的分辨率
    QList<QSize> sameSizeList = sameSizes.values();
    std::sort(sameSizeList.begin(), sameSizeList.end(), [](const QSize &s1, const QSize &s2) -> bool
              { return s1.width() * s1.height() < s2.width() * s2.height(); });
    const QSize biggestSize = sameSizeList.last();

    // Finally, look for the mode with biggestSize and biggest refreshRate and set it
    KLOG_DEBUG(display) << "Biggest Size: " << biggestSize;
    KScreen::ModePtr bestMode;
    for (auto monitor : connectedMonitors)
    {
        auto matchModes = monitor->getModesBySize(biggestSize.width(), biggestSize.height());
        if (matchModes.isEmpty())
        {
            KLOG_WARNING(display) << "Cannot match mode" << biggestSize << "for monitor" << monitor->getName();
            continue;
        }
        monitor->setEnabled(true);
        monitor->setX(0);
        monitor->setY(0);
        monitor->setRotation(uint16_t(DisplayRotationType::DISPLAY_ROTATION_0));
        monitor->setReflect(uint16_t(DisplayReflectType::DISPLAY_REFLECT_NORMAL));
        monitor->setCurrentMode(matchModes[0]->id().toInt());
    }

    return true;
}

void DisplayManager::switchToExtend()
{
    int32_t startx = 0;
    for (auto monitor : m_monitors)
    {
        if (!monitor->getConnected())
        {
            continue;
        }

        auto bestMode = monitor->getBestMode();
        if (!bestMode)
        {
            KLOG_WARNING(display) << "Failed to get best mode for monitor" << monitor->getName();
            continue;
        }

        monitor->setEnabled(true);
        monitor->setX(startx);
        monitor->setY(0);
        monitor->setCurrentMode(bestMode->id().toInt());
        monitor->setRotation(uint16_t(DisplayRotationType::DISPLAY_ROTATION_0));
        monitor->setReflect(uint16_t(DisplayReflectType::DISPLAY_REFLECT_NORMAL));

        startx += bestMode->size().width();
    }
}

bool DisplayManager::switchToCustom(CCErrorCode &errorCode)
{
    return applyConfig(errorCode);
}

void DisplayManager::switchToAuto()
{
    CCErrorCode error_code;

    RETURN_IF_TRUE(switchToCustom(error_code));
    switchToExtend();
}

// QSharedPointer<DisplayMonitor> DisplayManager::get_monitor(uint32_t id)
// {
//     auto iter = monitors_.find(id);
//     if (iter != monitors_.end())
//     {
//         return iter->second;
//     }
//     return nullptr;
// }

// QSharedPointer<DisplayMonitor> DisplayManager::get_monitor_by_uid(const QString &uid)
// {
//     for (const auto &iter : monitors_)
//     {
//         if (iter.second->get_uid() == uid)
//         {
//             return iter.second;
//         }
//     }
//     return nullptr;
// }

QSharedPointer<DisplayMonitor> DisplayManager::getMonitorByName(const QString &name)
{
    for (const auto &monitor : m_monitors)
    {
        if (monitor->getName() == name)
        {
            return monitor;
        }
    }
    return nullptr;
}

QSharedPointer<DisplayMonitor> DisplayManager::matchBestMonitor(const QString &uid, const QString &name)
{
    QSharedPointer<DisplayMonitor> retval;
    for (const auto &monitor : m_monitors)
    {
        if (!retval && monitor->getUID() == uid)
        {
            retval = monitor;
        }

        // 完美匹配则直接退出
        if (monitor->getUID() == uid && monitor->getName() == name)
        {
            retval = monitor;
            break;
        }
    }
    return retval;
}

QString DisplayManager::getMonitorsUID()
{
    QStringList result;
    for (const auto &monitor : m_monitors)
    {
        result.push_back(monitor->getUID());
    }
    std::sort(result.begin(), result.end(), std::less<QString>());
    return result.join(MONITOR_JOIN_CHAR);
}

QString DisplayManager::getMonitorNames()
{
    QStringList result;
    for (const auto &monitor : m_monitors)
    {
        result.push_back(monitor->getName());
    }
    std::sort(result.begin(), result.end(), std::less<QString>());
    return result.join(MONITOR_JOIN_CHAR);
}

QString DisplayManager::getCMonitorsUID(const ScreenConfigInfo::MonitorSequence &monitors)
{
    QStringList result;
    for (const auto &monitor : monitors)
    {
        result.push_back(monitor.uid().data());
    }
    std::sort(result.begin(), result.end(), std::less<QString>());
    return result.join(MONITOR_JOIN_CHAR);
}

bool DisplayManager::saveToFile(CCErrorCode &errorCode)
{
    // 文件不存在则先尝试创建对应的目录
    if (!QFileInfo::exists(m_configFilePath))
    {
        auto configDir = QFileInfo(m_configFilePath).dir();
        if (!configDir.mkpath(configDir.path()))
        {
            errorCode = CCErrorCode::ERROR_DISPLAY_SAVE_CREATE_FILE_FAILED;
            KLOG_WARNING(display) << "Failed to create directory" << configDir.path();
            return false;
        }
    }

    try
    {
        std::ofstream ofs(m_configFilePath.toLatin1().data(), std::ios_base::out);
        display(ofs, *m_displayConfig.get());
        ofs.close();
    }
    catch (const xml_schema::Exception &e)
    {
        KLOG_WARNING(display) << e.what();
        errorCode = CCErrorCode::ERROR_DISPLAY_WRITE_CONF_FILE_FAILED;
        return false;
    }
    return true;
}

void DisplayManager::processConfigureChanged()
{
    auto oldMonitorsUID = getMonitorsUID();
    auto oldMonitorNames = getMonitorNames();
    loadMonitors();
    auto newMonitorsUID = getMonitorsUID();
    auto newMonitorNames = getMonitorNames();

    auto screenChangedAdaptation = m_displaySettings->get(DISPLAY_SCHEMA_SCREEN_CHANGED_ADAPT).toBool();

    KLOG_INFO(display) << "check display resource changed, "
                       << "the old monitor uid and names is" << oldMonitorsUID << "/" << oldMonitorNames
                       << ", the new monitor uid and names is" << newMonitorsUID << "/" << newMonitorNames;

    /* 如果uid不相同且连接的接口不一样，说明设备硬件发生了变化，此时需要重新进行设置。这里不能只判断uid是否变化，因为有可能出现 */
    if (screenChangedAdaptation &&
        (oldMonitorsUID != newMonitorsUID || oldMonitorNames != newMonitorNames))
    {
        CCErrorCode errorCode = CCErrorCode::SUCCESS;
        if (!switchStyleAndSave(m_defaultStyle, errorCode))
        {
            KLOG_WARNING(display) << KCD_ERROR2STR(errorCode);
        }
    }

    Q_EMIT MonitorsChanged(true);
}

void DisplayManager::processSettingsChanged(const QString &key)
{
    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(DISPLAY_SCHEMA_STYLE, _hash):
    {
        auto style = DisplayStyle(styleStr2Enum(m_displaySettings->get(key).toString()));
        setDefaultStyle(style);
    }
    break;
    default:
        break;
    }
}

QString DisplayManager::styleEnum2Str(int style)
{
    switch (style)
    {
    case DisplayStyle::DISPLAY_STYLE_MIRRORS:
        return "mirrors";
    case DisplayStyle::DISPLAY_STYLE_EXTEND:
        return "extend";
    case DisplayStyle::DISPLAY_STYLE_CUSTOM:
        return "custom";
    case DisplayStyle::DISPLAY_STYLE_AUTO:
        return "auto";
    default:
        KLOG_WARNING(display) << "Unknown style enum value" << style << ", use auto style instead.";
        return "auto";
    }
    return "auto";
}

int DisplayManager::styleStr2Enum(QString style)
{
    switch (shash(style.toUtf8().data()))
    {
    case "mirrors"_hash:
        return DisplayStyle::DISPLAY_STYLE_MIRRORS;
    case "extend"_hash:
        return DisplayStyle::DISPLAY_STYLE_EXTEND;
    case "custom"_hash:
        return DisplayStyle::DISPLAY_STYLE_CUSTOM;
    case "auto"_hash:
        return DisplayStyle::DISPLAY_STYLE_AUTO;
    default:
        KLOG_WARNING(display) << "Unknown style" << style << ", use auto style instead.";
        break;
    }
    return DisplayStyle::DISPLAY_STYLE_AUTO;
}

}  // namespace Kiran
