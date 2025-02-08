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

#include <error-i.h>
#include <QDBusContext>
#include <QProcess>
#include <QString>
#include <QTranslator>
#include <QVector>
#include "dbus-types.h"

class TimeDateAdaptor;
class QDBusInterface;
class QFileSystemWatcher;
class QProcess;

namespace Kiran
{
class TimedateFormat;

struct ZoneInfo
{
    QString countryCode;
    QString coordinates;
    QString tz;
};

class TimedateManager : public QObject,
                        protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(bool can_ntp READ getCanNTP)
    Q_PROPERTY(int date_long_format_index READ getDateLongFormatIndex WRITE setDateLongFormatIndex)
    Q_PROPERTY(int date_short_format_index READ getDateShortFormatIndex WRITE setDateShortFormatIndex)
    Q_PROPERTY(int hour_format READ getHourFormat WRITE setHourFormat)
    Q_PROPERTY(bool local_rtc READ getLocalRtc WRITE setLocalRtc)
    Q_PROPERTY(bool ntp READ getNTP WRITE setNTP)
    Q_PROPERTY(qulonglong rtc_time READ getRtcTime)
    Q_PROPERTY(bool seconds_showing READ getSecondsShowing WRITE setSecondsShowing)
    Q_PROPERTY(qulonglong system_time READ getSystemTime)
    Q_PROPERTY(QString time_zone READ getTimeZone WRITE setTimeZone)

public:
    TimedateManager();
    virtual ~TimedateManager();

    static TimedateManager *get_instance() { return m_instance; };

    static void globalInit();
    static void globalDeinit() { delete m_instance; };

public:
    bool getCanNTP() const { return m_ntpUnitName.length() > 0; };
    int getDateLongFormatIndex() const;
    int getDateShortFormatIndex() const;
    int getHourFormat() const;
    bool getLocalRtc() const { return m_localRtc; };
    bool getNTP() const { return m_ntpActive; };
    qulonglong getRtcTime() const;
    bool getSecondsShowing() const;
    qulonglong getSystemTime() const;
    QString getTimeZone() const { return m_timeZone; };

    void setDateLongFormatIndex(int index);
    void setDateShortFormatIndex(int index);
    void setHourFormat(int format);
    void setLocalRtc(bool rtc);
    void setNTP(bool active);
    void setSecondsShowing(bool secondsShowing);
    void setTimeZone(const QString &timeZone);

public Q_SLOTS:
    // 开启时间显示到秒
    void EnableSecondsShowing(bool enabled);
    // 获取可设置的日期时间格式列表，type参考TimedateFormatType
    QStringList GetDateFormatList(int type);
    // 获取时区列表
    DBusZoneInfos GetZoneList();
    // 通过索引设置日期时间格式
    void SetDateFormatByIndex(int type, int index);
    // 设置小时显示格式，format参考TimedateHourFormat
    void SetHourFormat(int format);
    // 调整硬件时钟设置，如果local为true则按本地时区进行存储，否则按UTC存储，adjust_system为true则将硬件时间同步到系统时间，否则将系统时间同步到硬件时间
    void SetLocalRTC(bool local, bool adjustSystem);
    // 是否开启网络时间同步
    void SetNTP(bool active);
    // 设置系统(软件)时间，如果relative为真，则设置相对时间，否则为绝对时间，时间单位为微妙
    void SetTime(qlonglong requestedTime, bool relative);
    // 设置系统时区
    void SetTimezone(const QString &timeZone);

private:
    void init();
    void initNTPUnits();
    // 获取可用的时间同步服务
    QStringList getNTPUnits();
    // 开启NTP服务
    bool startNTPUnit(const QString &name, CCErrorCode &errorCode);
    // 停止NTP服务
    bool stopNTPUnit(const QString &name, CCErrorCode &errorCode);
    // NTP服务是否开启
    bool ntpIsActive(const QString &name);
    QString getUnitObjectPath(const QString &name);

    QVector<ZoneInfo> getZoneInfos();

    QDBusMessage callSystemd(const QString &methodName, const QList<QVariant> &arguments);
    bool callSystemdNoresult(const QString &methodName, const QList<QVariant> &arguments);

    bool syncHWClock(bool hctosys, bool local, bool utc);
    void funishSetTime(const QDBusMessage &message, int64_t requestTime, int64_t requestedTime, bool relative);

    void setLocaltimeFileContext(const QString &path);
    void updateKernelUtcOffset();
    bool checkTimezoneName(const QString &name);
    void finishSetTimezone(const QDBusMessage &message, const QString &timeZone);
    void finishSetRtcLocal(const QDBusMessage &message, bool local, bool adjustSystem);
    void finishSetNTPActive(const QDBusMessage &message, bool active);

private Q_SLOTS:
    // 监控信号变化处理
    void processNTPUnitPropsChanged(const QDBusMessage &message);
    void processFilesChanged(const QString &path);
    void processDirectoryChanged(const QString &path);
    void processHWSyncFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    static TimedateManager *m_instance;

    TimeDateAdaptor *m_adaptor;
    TimedateFormat *m_timedateFormat;
    QDBusInterface *m_ntpUnitInterface;
    QFileSystemWatcher *m_fileWatcher;
    QTranslator *m_tzTranslator;
    const static QStringList m_ntpUnitsPaths;
    QString m_ntpUnitName;
    QString m_timeZone;
    bool m_localRtc;
    bool m_ntpActive;
    QProcess *m_hwSyncProcess;
};
}  // namespace Kiran