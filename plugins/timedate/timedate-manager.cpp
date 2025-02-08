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

#include "timedate-manager.h"
#include <fcntl.h>
#include <glib.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <unistd.h>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QFile>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QRandomGenerator>
#include <QRegExp>
#include "config.h"
#include "lib/base/base.h"
#include "lib/base/polkit-proxy.h"
#include "timedate-def.h"
#include "timedate-format.h"
#include "timedate-i.h"
#include "timedate-util.h"
#include "timedateadaptor.h"

#ifdef HAVE_SELINUX
#include <selinux/selinux.h>
#endif

/* This may be missing in libc headers */
#ifndef ADJ_SETOFFSET
#define ADJ_SETOFFSET 0x0100
#endif

namespace Kiran
{
#define HWCLOCK_PATH "/sbin/hwclock"
#define RTC_DEVICE "/dev/rtc"

#define ZONE_TABLE "zone.tab"
#define ZONE_TABLE_MIN_COLUMN 3
#define LOCALTIME_TO_ZONEINFO_PATH ".."
#define MAX_TIMEZONE_LENGTH 256

#define UNIT_PROP_ACTIVE_STATE "ActiveState"

#define TIMEZONE_DOMAIN "kiran-cc-daemon-timezones"

#define POLKIT_ACTION_SET_TIME "com.kylinsec.kiran.system-daemon.timedate.set-time"
#define POLKIT_ACTION_SET_NTP_ACTIVE "com.kylinsec.kiran.system-daemon.timedate.set-ntp"
#define POLKIT_ACTION_SET_RTC_LOCAL "com.kylinsec.kiran.system-daemon.timedate.set-local-rtc"
#define POLKIT_ACTION_SET_TIMEZONE "com.kylinsec.kiran.system-daemon.timedate.set-timezone"
#define POLKIT_AUTH_CHECK_TIMEOUT 20

TimedateManager *TimedateManager::m_instance = nullptr;
const QStringList TimedateManager::m_ntpUnitsPaths = {"/etc/systemd/ntp-units.d", "/usr/lib/systemd/ntp-units.d"};

TimedateManager::TimedateManager() : m_ntpUnitInterface(nullptr),
                                     m_localRtc(false),
                                     m_ntpActive(false)
{
    m_adaptor = new TimeDateAdaptor(this);
    m_timedateFormat = new TimedateFormat(this);
    m_fileWatcher = new QFileSystemWatcher(this);
    m_tzTranslator = new QTranslator(this);
    m_hwSyncProcess = new QProcess(this);

    qDBusRegisterMetaType<DBusZoneInfo>();
    qDBusRegisterMetaType<DBusZoneInfos>();
}

TimedateManager::~TimedateManager()
{
}

void TimedateManager::globalInit()
{
    m_instance = new TimedateManager();
    m_instance->init();
}

int TimedateManager::getDateLongFormatIndex() const
{
    return m_timedateFormat->getDateLongFormatIndex();
}

int TimedateManager::getDateShortFormatIndex() const
{
    return m_timedateFormat->getDateShortFormatIndex();
}

int TimedateManager::getHourFormat() const
{
    return m_timedateFormat->getHourFormat();
}

qulonglong TimedateManager::getRtcTime() const
{
    struct rtc_time rtc;
    struct tm tm;
    time_t rtc_time = 0;
    int fd, r;

    fd = open(RTC_DEVICE, O_RDONLY);
    if (fd < 0)
    {
        return 0;
    }

    r = ioctl(fd, RTC_RD_TIME, &rtc);
    close(fd);
    if (r)
    {
        return 0;
    }

    tm.tm_sec = rtc.tm_sec;
    tm.tm_min = rtc.tm_min;
    tm.tm_hour = rtc.tm_hour;
    tm.tm_mday = rtc.tm_mday;
    tm.tm_mon = rtc.tm_mon;
    tm.tm_year = rtc.tm_year;
    tm.tm_isdst = 0;

    /* This is the raw time as if the RTC was in UTC */
    rtc_time = timegm(&tm);

    return (qulonglong)rtc_time * 1000000;
}

bool TimedateManager::getSecondsShowing() const
{
    return m_timedateFormat->getSecondsShowing();
}

qulonglong TimedateManager::getSystemTime() const
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (qulonglong)tv.tv_sec * 1000000 + tv.tv_usec;
}

#define SEND_PROPERTY_NOTIFY(property, propertyHump)                          \
    QVariantMap changedProperties;                                            \
    changedProperties.insert(QStringLiteral(#property), get##propertyHump()); \
                                                                              \
    QDBusMessage signalMessage = QDBusMessage::createSignal(                  \
        TIMEDATE_OBJECT_PATH,                                                 \
        QStringLiteral("org.freedesktop.DBus.Properties"),                    \
        QStringLiteral("PropertiesChanged"));                                 \
                                                                              \
    signalMessage.setArguments({                                              \
        TIMEDATE_DBUS_INTERFACE_NAME,                                         \
        changedProperties,                                                    \
        QStringList(),                                                        \
    });                                                                       \
    QDBusConnection::systemBus().send(signalMessage);

void TimedateManager::setDateLongFormatIndex(int index)
{
    if (m_timedateFormat->setDateLongFormat(index))
    {
        SEND_PROPERTY_NOTIFY(date_long_format_index, DateLongFormatIndex);
    }
}

void TimedateManager::setDateShortFormatIndex(int index)
{
    if (m_timedateFormat->setDateShortFormat(index))
    {
        SEND_PROPERTY_NOTIFY(date_short_format_index, DateShortFormatIndex);
    }
}

void TimedateManager::setHourFormat(int format)
{
    RETURN_IF_TRUE(format < 0 || format >= TimedateHourFormat::TIMEDATE_HOUSR_FORMAT_LAST);

    if (m_timedateFormat->setHourFormat(TimedateHourFormat(format)))
    {
        SEND_PROPERTY_NOTIFY(hour_format, HourFormat);
    }
}

void TimedateManager::setLocalRtc(bool rtc)
{
    if (m_localRtc != rtc)
    {
        m_localRtc = rtc;
        SEND_PROPERTY_NOTIFY(local_rtc, LocalRtc);
    }
}

void TimedateManager::setNTP(bool active)
{
    if (m_ntpActive != active)
    {
        m_ntpActive = active;
        SEND_PROPERTY_NOTIFY(ntp, NTP);
    }
}

void TimedateManager::setSecondsShowing(bool secondsShowing)
{
    m_timedateFormat->setSecondsShowing(secondsShowing);
    SEND_PROPERTY_NOTIFY(seconds_showing, SecondsShowing);
}

void TimedateManager::setTimeZone(const QString &timeZone)
{
    if (m_timeZone != timeZone)
    {
        m_timeZone = timeZone;
        SEND_PROPERTY_NOTIFY(time_zone, TimeZone);
    }
}

void TimedateManager::EnableSecondsShowing(bool enabled)
{
    setSecondsShowing(enabled);
}

QStringList TimedateManager::GetDateFormatList(int type)
{
    switch (type)
    {
    case TimedateDateFormatType::TIMEDATE_FORMAT_TYPE_LONG:
        return m_timedateFormat->getLongFormats();
    case TimedateDateFormatType::TIMEDATE_FORMAT_TYPE_SHORT:
        return m_timedateFormat->getShortFormats();
    default:
        DBUS_ERROR_REPLY_AND_RETVAL(QStringList(), CCErrorCode::ERROR_TIMEDATE_UNKNOWN_DATE_FORMAT_TYPE_1);
    }
    return QStringList();
}

DBusZoneInfos TimedateManager::GetZoneList()
{
    DBusZoneInfos dbusZoneInfos;

    auto zoneInfos = getZoneInfos();
    for (auto &zoneInfo : zoneInfos)
    {
        auto localZoneName = m_tzTranslator->translate("TimeZone", zoneInfo.tz.toLatin1().data());
        auto gmt = TimedateUtil::getGMTOffset(zoneInfo.tz);

        DBusZoneInfo dbusZoneInfo;
        dbusZoneInfo.name = zoneInfo.tz;
        dbusZoneInfo.localName = localZoneName;
        dbusZoneInfo.gmt = qlonglong(gmt);

        dbusZoneInfos.push_back(dbusZoneInfo);
    }

    return dbusZoneInfos;
}

void TimedateManager::SetDateFormatByIndex(int type, int index)
{
    switch (type)
    {
    case TimedateDateFormatType::TIMEDATE_FORMAT_TYPE_LONG:
        setDateLongFormatIndex(index);
        break;
    case TimedateDateFormatType::TIMEDATE_FORMAT_TYPE_SHORT:
        setDateShortFormatIndex(index);
        break;
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_TIMEDATE_UNKNOWN_DATE_FORMAT_TYPE_2);
    }
}

void TimedateManager::SetHourFormat(int format)
{
    setHourFormat(format);
}

void TimedateManager::SetLocalRTC(bool local, bool adjustSystem)
{
    RETURN_IF_TRUE(local == getLocalRtc())

    this->setDelayedReply(true);
    PolkitProxy::getDefault()->checkAuthorization(POLKIT_ACTION_SET_RTC_LOCAL,
                                                  true,
                                                  this->message(),
                                                  QStringLiteral("TimedateManager::SetLocalRTC"),
                                                  std::bind(&TimedateManager::finishSetRtcLocal, this, std::placeholders::_1, local, adjustSystem));
}

void TimedateManager::SetNTP(bool active)
{
    RETURN_IF_TRUE(active == getNTP());

    if (m_ntpUnitName.length() == 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_TIMEDATE_NO_NTP_UNIT);
    }

    this->setDelayedReply(true);
    PolkitProxy::getDefault()->checkAuthorization(POLKIT_ACTION_SET_NTP_ACTIVE,
                                                  true,
                                                  this->message(),
                                                  QStringLiteral("TimedateManager::SetNTP"),
                                                  std::bind(&TimedateManager::finishSetNTPActive, this, std::placeholders::_1, active));
}

void TimedateManager::SetTime(qlonglong requestedTime, bool relative)
{
    if (getNTP())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_TIMEDATE_NTP_IS_ACTIVE);
    }

    int64_t requestTime = g_get_monotonic_time();

    this->setDelayedReply(true);
    PolkitProxy::getDefault()->checkAuthorization(POLKIT_ACTION_SET_TIME,
                                                  true,
                                                  this->message(),
                                                  QStringLiteral("TimedateManager::SetTime"),
                                                  std::bind(&TimedateManager::funishSetTime, this, std::placeholders::_1, requestTime, requestedTime, relative));
}

void TimedateManager::SetTimezone(const QString &timeZone)
{
    if (!checkTimezoneName(timeZone))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_TIMEDATE_TIMEZONE_INVALIDE);
    }

    auto currentTimezone = getTimeZone();
    RETURN_IF_TRUE(currentTimezone == timeZone);

    this->setDelayedReply(true);
    PolkitProxy::getDefault()->checkAuthorization(POLKIT_ACTION_SET_TIMEZONE,
                                                  true,
                                                  this->message(),
                                                  QStringLiteral("TimedateManager::SetTimezone"),
                                                  std::bind(&TimedateManager::finishSetTimezone, this, std::placeholders::_1, timeZone));
}

void TimedateManager::init()
{
    auto translationFilePath = QString("%1-%2").arg(PROJECT_NAME).arg("timezones");
    if (!m_tzTranslator->load(QLocale(), translationFilePath, ".", KCD_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING(timedate) << "Load translation file timezones failed.";
    }

    auto systemConnection = QDBusConnection::systemBus();
    if (!systemConnection.registerService(TIMEDATE_DBUS_NAME))
    {
        KLOG_WARNING(timedate) << "Failed to register dbus name: " << TIMEDATE_DBUS_NAME;
        return;
    }

    if (!systemConnection.registerObject(TIMEDATE_OBJECT_PATH, TIMEDATE_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR(timedate) << "Can't register object:" << systemConnection.lastError();
        return;
    }

    m_fileWatcher->addPath(LOCALTIME_PATH);
    m_fileWatcher->addPath(ADJTIME_PATH);

    for (const auto &unitDir : m_ntpUnitsPaths)
    {
        m_fileWatcher->addPath(unitDir);
    }

    m_timeZone = TimedateUtil::getTimezone();
    m_localRtc = TimedateUtil::isLocalRtc();
    initNTPUnits();
    m_ntpActive = ntpIsActive(m_ntpUnitName);
    m_timedateFormat->init();

    connect(m_fileWatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(processFilesChanged(const QString &)));
    connect(m_fileWatcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(processDirectoryChanged(const QString &)));
    connect(m_hwSyncProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processHWSyncFinished(int, QProcess::ExitStatus)));
}

void TimedateManager::initNTPUnits()
{
    auto ntpUnits = getNTPUnits();
    CCErrorCode errorCode = CCErrorCode::SUCCESS;

    // 优先选择已经激活的NTP服务，如果都没有激活则选择第一个NTP服务
    m_ntpUnitName.clear();
    for (auto &ntpUnit : ntpUnits)
    {
        if (ntpIsActive(ntpUnit))
        {
            m_ntpUnitName = ntpUnit;
            break;
        }
    }

    if (m_ntpUnitName.isEmpty())
    {
        m_ntpUnitName = ntpUnits.front();
    }

    KLOG_INFO(timedate) << "Use" << m_ntpUnitName << "as default NTP service, other NTP service will be stopped.";

    // 关闭掉其他NTP服务，避免冲突
    for (auto &ntpUnit : ntpUnits)
    {
        CONTINUE_IF_TRUE(ntpUnit == m_ntpUnitName)
        stopNTPUnit(ntpUnit, errorCode);
    }

    auto unitObjectPath = getUnitObjectPath(m_ntpUnitName);

    if (unitObjectPath.length() > 0)
    {
        m_ntpUnitInterface = new QDBusInterface(SYSTEMD_NAME,
                                                unitObjectPath,
                                                SYSTEMD_UNIT_INTERFACE,
                                                QDBusConnection::systemBus(),
                                                this);
        QDBusConnection::systemBus().connect(SYSTEMD_NAME,
                                             unitObjectPath,
                                             QStringLiteral("org.freedesktop.DBus.Properties"),
                                             "PropertiesChanged",
                                             this,
                                             SLOT(processNTPUnitPropsChanged(const QDBusMessage &)));
    }
}

QStringList TimedateManager::getNTPUnits()
{
    QStringList ntpUnits;

    for (auto &ntpUnitsPath : m_ntpUnitsPaths)
    {
        QDir dir(ntpUnitsPath);
        auto files = dir.entryList(QDir::Files, QDir::Name | QDir::Reversed);

        for (auto &fileName : files)
        {
            auto filePath = QString("%1/%2").arg(ntpUnitsPath).arg(fileName);
            QFile file(filePath);

            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                KLOG_WARNING(timedate) << "Cannot access file" << filePath;
                continue;
            }

            auto contents = file.readAll();
            auto lines = contents.split('\n');
            for (auto &line : lines)
            {
                if (line.length() == 0 || line.at(0) == '#')
                {
                    KLOG_DEBUG(timedate) << "The line" << line << "is ingored.";
                    continue;
                }
                auto arguments = QList<QVariant>{QString(line)};
                if (!callSystemdNoresult("LoadUnit", arguments))
                {
                    KLOG_WARNING(timedate) << "Failed to LoadUnit" << line;
                    continue;
                }

                if (!ntpUnits.contains(line))
                {
                    ntpUnits.push_back(line);
                }
                else
                {
                    KLOG_WARNING(timedate) << "Ignore duplication ntp unit" << line;
                }
            }
        }
    }

    return ntpUnits;
}

bool TimedateManager::startNTPUnit(const QString &name, CCErrorCode &errorCode)
{
    KLOG_INFO(timedate) << "Start and enable NTP service" << name;

    auto startUnitArguments = QList<QVariant>{name, QString("replace")};
    if (!callSystemdNoresult("StartUnit", startUnitArguments))
    {
        errorCode = CCErrorCode::ERROR_TIMEDATE_START_NTP_FAILED;
        return false;
    }

    auto enableUnitArguments = QList<QVariant>{QStringList(name), false, true};
    callSystemdNoresult("EnableUnitFiles", enableUnitArguments);
    callSystemdNoresult("Reload", QList<QVariant>());
    return true;
}

bool TimedateManager::stopNTPUnit(const QString &name, CCErrorCode &errorCode)
{
    KLOG_INFO(timedate) << "Stop and disable NTP service" << name;

    auto stopUnitArguments = QList<QVariant>{name, QString("replace")};
    if (!callSystemdNoresult("StopUnit", stopUnitArguments))
    {
        errorCode = CCErrorCode::ERROR_TIMEDATE_STOP_NTP_FAILED;
        return false;
    }

    auto disableUnitArguments = QList<QVariant>{QStringList(name), false};
    callSystemdNoresult("DisableUnitFiles", disableUnitArguments);
    callSystemdNoresult("Reload", QList<QVariant>());
    return true;
}

bool TimedateManager::ntpIsActive(const QString &name)
{
    auto ntpObjectPath = getUnitObjectPath(name);
    RETURN_VAL_IF_TRUE(ntpObjectPath.size() <= 0, false);

    QDBusInterface ntpUnitInterface(SYSTEMD_NAME,
                                    ntpObjectPath,
                                    SYSTEMD_UNIT_INTERFACE,
                                    QDBusConnection::systemBus());

    auto state = ntpUnitInterface.property(UNIT_PROP_ACTIVE_STATE).toString();
    return (state == "active" || state == "activating");
}

QString TimedateManager::getUnitObjectPath(const QString &name)
{
    RETURN_VAL_IF_TRUE(name.size() <= 0, QString());

    auto arguments = QList<QVariant>{name};
    auto replyMessage = callSystemd("LoadUnit", arguments);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(timedate) << "Call LoadUnit failed: " << replyMessage.errorMessage();
        return QString();
    }

    auto retval = replyMessage.arguments().takeFirst().value<QDBusObjectPath>();
    return retval.path();
}

QVector<ZoneInfo> TimedateManager::getZoneInfos()
{
    QVector<ZoneInfo> zoneInfos;

    auto filePath = QString("%1%2").arg(ZONEINFO_PATH).arg(ZONE_TABLE);
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        KLOG_WARNING(timedate) << "Cannot access file " << filePath;
        return zoneInfos;
    }

    QString line;

    auto contents = file.readAll();
    auto lines = contents.split('\n');

    for (auto &line : lines)
    {
        if (line.length() == 0 || line.at(0) == '#')
        {
            continue;
        }

        auto fields = QString(line).split(QRegExp("\\s+"));
        if (fields.size() >= ZONE_TABLE_MIN_COLUMN)
        {
            ZoneInfo zoneInfo;
            zoneInfo.countryCode = fields[0];
            zoneInfo.coordinates = fields[1];
            zoneInfo.tz = fields[2];
            zoneInfos.push_back(std::move(zoneInfo));
        }
        else
        {
            KLOG_WARNING(timedate) << "Ignore line" << line << ", because of the line is less than" << ZONE_TABLE_MIN_COLUMN << "columns.";
        }
    }

    return zoneInfos;
}

QDBusMessage TimedateManager::callSystemd(const QString &methodName, const QList<QVariant> &arguments)
{
    QDBusMessage sendMessage = QDBusMessage::createMethodCall(SYSTEMD_NAME,
                                                              SYSTEMD_PATH,
                                                              SYSTEMD_MANAGER_INTERFACE,
                                                              methodName);
    sendMessage.setArguments(arguments);
    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block, DBUS_TIMEOUT_MS);
    return replyMessage;
}

bool TimedateManager::callSystemdNoresult(const QString &methodName, const QList<QVariant> &arguments)
{
    auto replyMessage = callSystemd(methodName, arguments);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(timedate) << "Call Get failed: " << replyMessage.errorMessage();
        return false;
    }
    return true;
}

bool TimedateManager::syncHWClock(bool hctosys, bool local, bool utc)
{
    struct stat st;
    if (stat(RTC_DEVICE, &st) || !(st.st_mode & S_IFCHR))
    {
        KLOG_WARNING(timedate) << "No RTC device";
        return false;
    }

    if (m_hwSyncProcess->state() != QProcess::NotRunning)
    {
        KLOG_WARNING(timedate) << "Exists a hwclock process running already";
        return true;
    }

    QStringList arguments{"-f", RTC_DEVICE};

    if (hctosys)
    {
        arguments.append("--hctosys");
    }
    else
    {
        arguments.append("--systohc");
    }

    if (local)
    {
        arguments.append("--local");
    }

    if (utc)
    {
        arguments.append("--utc");
    }

    auto command = QString("%1 %2").arg(HWCLOCK_PATH).arg(arguments.join(" "));
    KLOG_INFO(timedate) << "Execute" << command << "to sync hardware clock";

    m_hwSyncProcess->setProgram(HWCLOCK_PATH);
    m_hwSyncProcess->setArguments(arguments);
    m_hwSyncProcess->start();
    return true;
}

void TimedateManager::funishSetTime(const QDBusMessage &message, int64_t requestTime, int64_t requestedTime, bool relative)
{
    struct timeval tv;
    struct timex tx;
    QString errMessage;

    if (relative)
    {
        tx.modes = ADJ_SETOFFSET | ADJ_NANO;

        tx.time.tv_sec = requestedTime / 1000000;
        tx.time.tv_usec = requestedTime - tx.time.tv_sec * 1000000;
        if (tx.time.tv_usec < 0)
        {
            tx.time.tv_sec--;
            tx.time.tv_usec += 1000000;
        }

        /* Convert to nanoseconds */
        tx.time.tv_usec *= 1000;

        if (adjtimex(&tx) < 0)
        {
            errMessage = QString("Failed to set system clock: %1").arg(strerror(errno));
        }
    }
    else
    {
        /* Compensate for the time taken by the authorization check */
        requestedTime += g_get_monotonic_time() - requestTime;

        tv.tv_sec = requestedTime / 1000000;
        tv.tv_usec = requestedTime - tv.tv_sec * 1000000;
        if (settimeofday(&tv, NULL))
        {
            errMessage = QString("Failed to set system clock: %1").arg(strerror(errno));
        }
    }

    if (errMessage.length() > 0)
    {
        auto replyMessage = message.createErrorReply(QDBusError::Failed, errMessage);
        QDBusConnection::systemBus().send(replyMessage);
    }
    else
    {
        syncHWClock(false, false, false);
        QDBusConnection::systemBus().send(message.createReply());
    }
}

void TimedateManager::setLocaltimeFileContext(const QString &path)
{
#ifdef HAVE_SELINUX
    security_context_t con;

    if (!is_selinux_enabled())
        return;

    if (matchpathcon_init_prefix(NULL, LOCALTIME_PATH))
        return;

    if (!matchpathcon(LOCALTIME_PATH, S_IFLNK, &con))
    {
        lsetfilecon(path.toLatin1().data(), con);
        freecon(con);
    }

    matchpathcon_fini();
#endif
}

void TimedateManager::updateKernelUtcOffset(void)
{
    struct timezone tz;
    struct timeval tv;

    bool updated = false;

    do
    {
        struct tm *tm;
        if (gettimeofday(&tv, &tz))
        {
            break;
        }

        tm = localtime(&tv.tv_sec);
        if (!tm)
        {
            break;
        }

        /* tm_gmtoff is in seconds east of UTC */
        tz.tz_minuteswest = -tm->tm_gmtoff / 60;
        if (settimeofday(NULL, &tz))
        {
            break;
        }

        updated = true;
    } while (0);

    if (!updated)
    {
        KLOG_WARNING(timedate) << "Failed to update kernel UTC offset";
    }
}

bool TimedateManager::checkTimezoneName(const QString &name)
{
    /* Check if the name is sane */
    if (name.length() == 0 ||
        name.at(0) == '/' ||
        name.contains("//") ||
        name.contains("..") ||
        name.length() > MAX_TIMEZONE_LENGTH)
        return false;

    for (auto c : name)
    {
        if (!c.isLetterOrNumber() && !strchr("+-_/", c.toLatin1()))
        {
            return false;
        }
    }

    /* Check if the correspoding file exists in the zoneinfo directory, it
           doesn't have to be listed in zone.tab */
    auto link = QString("%1%2").arg(ZONEINFO_PATH).arg(name);
    struct stat st;
    if (stat(link.toLatin1().data(), &st) || !(st.st_mode & S_IFREG))
        return false;

    return true;
}

void TimedateManager::finishSetTimezone(const QDBusMessage &message, const QString &timeZone)
{
    auto link = QString("%1%2%3").arg(LOCALTIME_TO_ZONEINFO_PATH).arg(ZONEINFO_PATH).arg(timeZone);
    auto tmp = QString("%1.%2").arg(LOCALTIME_PATH).arg(QRandomGenerator(time(NULL)).generate());
    bool successed = false;
    do
    {
        KLOG_INFO(timedate) << "Set symbol link of" << LOCALTIME_PATH << "to" << link;

        if (symlink(link.toLatin1().data(), tmp.toLatin1().data())) break;

        setLocaltimeFileContext(tmp);

        if (rename(tmp.toLatin1().data(), LOCALTIME_PATH))
        {
            unlink(tmp.toLatin1().data());
            break;
        }

        setTimeZone(timeZone);

        updateKernelUtcOffset();

        /* RTC in local needs to be set for the new timezone */
        if (getLocalRtc())
        {
            syncHWClock(false, false, false);
        }
        successed = true;
    } while (0);

    if (!successed)
    {
        auto replyMessage = message.createErrorReply(QDBusError::Failed, "Failed to update " LOCALTIME_PATH);
        QDBusConnection::systemBus().send(replyMessage);
    }
    else
    {
        QDBusConnection::systemBus().send(message.createReply());
    }
}

void TimedateManager::finishSetRtcLocal(const QDBusMessage &message, bool local, bool adjustSystem)
{
    if (syncHWClock(adjustSystem, local, !local))
    {
        setLocalRtc(local);
        QDBusConnection::systemBus().send(message.createReply());
    }
    else
    {
        DBUS_ERROR_DELAY_REPLY(CCErrorCode::ERROR_FAILED);
    }
}

void TimedateManager::finishSetNTPActive(const QDBusMessage &message, bool active)
{
    CCErrorCode errorCode = CCErrorCode::SUCCESS;
    bool result = false;
    if (active)
    {
        result = startNTPUnit(m_ntpUnitName, errorCode);
    }
    else
    {
        result = stopNTPUnit(m_ntpUnitName, errorCode);
    }

    if (!result)
    {
        DBUS_ERROR_DELAY_REPLY_AND_RET(errorCode);
    }

    setNTP(active);
    QDBusConnection::systemBus().send(message.createReply());
}

void TimedateManager::processNTPUnitPropsChanged(const QDBusMessage &message)
{
    QList<QVariant> args = message.arguments();
    RETURN_IF_TRUE(args.count() != 3);

    QVariantMap changedProperties = qdbus_cast<QVariantMap>(args.at(1).value<QDBusArgument>());
    auto iter = changedProperties.find(UNIT_PROP_ACTIVE_STATE);
    RETURN_IF_TRUE(iter == changedProperties.end());
    auto state = iter.value().toString();

    if (state == "active" || state == "activating")
    {
        setNTP(true);
    }
    else
    {
        setNTP(false);
    }
}

void TimedateManager::processFilesChanged(const QString &path)
{
    switch (shash(path.toLatin1().data()))
    {
    case CONNECT(LOCALTIME_PATH, _hash):
    {
        auto tz = TimedateUtil::getTimezone();
        setTimeZone(tz);
        break;
    }
    case CONNECT(ADJTIME_PATH, _hash):
        setLocalRtc(TimedateUtil::isLocalRtc());
        break;
    }
}

void TimedateManager::processDirectoryChanged(const QString &path)
{
    initNTPUnits();
}

void TimedateManager::processHWSyncFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::ExitStatus::NormalExit)
    {
        KLOG_WARNING(timedate) << "hwclock sync failed, exit code is" << exitCode << ", exit status is" << exitStatus;
    }
}

}  // namespace Kiran