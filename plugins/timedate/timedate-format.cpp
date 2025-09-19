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

#include "timedate-format.h"
#include <QSettings>
#include "lib/base/base.h"

namespace Kiran
{
#define TIMEDATE_FORMAT_CONFIG_DIR "/etc/kiran-cc-daemon/system/timedate/"
#define TIMEDATE_FORMAT_CONFIG_FILENAME "timedate.conf"

#define TIMEDATE_FORMAT_GROUP_NAME "format"
#define TIMEDATE_FORMAT_KEY_DATE_LONG_FORMAT_INDEX "date_long_format_index"
#define TIMEDATE_FORMAT_KEY_DATE_SHORT_FORMAT_INDEX "date_short_format_index"
#define TIMEDATE_FORMAT_KEY_HOUR_FORMAT "hour_format"
#define TIMEDATE_FORMAT_KEY_SECONDS_SHOWING "seconds_showing"

TimedateFormat::TimedateFormat(QObject *parent) : QObject(parent)
{
    auto fileName = QString("%1%2").arg(TIMEDATE_FORMAT_CONFIG_DIR).arg(TIMEDATE_FORMAT_CONFIG_FILENAME);
    m_settings = new QSettings(fileName, QSettings::IniFormat, this);
}

TimedateFormat::~TimedateFormat()
{
}

void TimedateFormat::init()
{
}

int32_t TimedateFormat::getDateLongFormatIndex()
{
    auto key = QString("%1/%2").arg(TIMEDATE_FORMAT_GROUP_NAME).arg(TIMEDATE_FORMAT_KEY_DATE_LONG_FORMAT_INDEX);
    auto longFormatIndex = m_settings->value(key).toInt();

    // 如果索引超出范围，则设置为第一个
    RETURN_VAL_IF_TRUE(longFormatIndex < 0 || longFormatIndex >= (int32_t)getLongFormats().size(), 0);
    return longFormatIndex;
}

int32_t TimedateFormat::getDateShortFormatIndex()
{
    auto key = QString("%1/%2").arg(TIMEDATE_FORMAT_GROUP_NAME).arg(TIMEDATE_FORMAT_KEY_DATE_SHORT_FORMAT_INDEX);
    auto shortFormatIndex = m_settings->value(key).toInt();

    // 如果索引超出范围，则设置为第一个
    RETURN_VAL_IF_TRUE(shortFormatIndex < 0 || shortFormatIndex >= (int32_t)getShortFormats().size(), 0);
    return shortFormatIndex;
}

TimedateHourFormat TimedateFormat::getHourFormat()
{
    auto key = QString("%1/%2").arg(TIMEDATE_FORMAT_GROUP_NAME).arg(TIMEDATE_FORMAT_KEY_HOUR_FORMAT);
    auto hourFormat = m_settings->value(key).toInt();

    // 如果索引超出范围，则设置为12小时制
    RETURN_VAL_IF_TRUE(hourFormat < 0 || hourFormat >= TimedateHourFormat::TIMEDATE_HOUSR_FORMAT_LAST,
                       TimedateHourFormat::TIMEDATE_HOUSR_FORMAT_12_HOURS);
    return TimedateHourFormat(hourFormat);
}

bool TimedateFormat::getSecondsShowing()
{
    auto key = QString("%1/%2").arg(TIMEDATE_FORMAT_GROUP_NAME).arg(TIMEDATE_FORMAT_KEY_SECONDS_SHOWING);
    return m_settings->value(key).toBool();
}

bool TimedateFormat::setDateLongFormat(int32_t index)
{
    RETURN_VAL_IF_TRUE(index < 0 || index >= (int32_t)getLongFormats().size(), false);
    RETURN_VAL_IF_TRUE(index == getDateLongFormatIndex(), true);

    auto key = QString("%1/%2").arg(TIMEDATE_FORMAT_GROUP_NAME).arg(TIMEDATE_FORMAT_KEY_DATE_LONG_FORMAT_INDEX);
    m_settings->setValue(key, index);
    KLOG_INFO(timedate) << "Set date long format to index" << index;
    return true;
}

bool TimedateFormat::setDateShortFormat(int32_t index)
{
    RETURN_VAL_IF_TRUE(index < 0 || index >= (int32_t)getShortFormats().size(), false);
    RETURN_VAL_IF_TRUE(index == getDateShortFormatIndex(), true);

    auto key = QString("%1/%2").arg(TIMEDATE_FORMAT_GROUP_NAME).arg(TIMEDATE_FORMAT_KEY_DATE_SHORT_FORMAT_INDEX);
    m_settings->setValue(key, index);
    KLOG_INFO(timedate) << "Set date short format to index" << index;
    return true;
}

bool TimedateFormat::setHourFormat(TimedateHourFormat hourFormat)
{
    auto key = QString("%1/%2").arg(TIMEDATE_FORMAT_GROUP_NAME).arg(TIMEDATE_FORMAT_KEY_HOUR_FORMAT);
    m_settings->setValue(key, int(hourFormat));
    KLOG_INFO(timedate) << "Set hour format to index" << hourFormat;
    return true;
}

bool TimedateFormat::setSecondsShowing(bool secondsShowing)
{
    auto key = QString("%1/%2").arg(TIMEDATE_FORMAT_GROUP_NAME).arg(TIMEDATE_FORMAT_KEY_SECONDS_SHOWING);
    m_settings->setValue(key, secondsShowing);
    KLOG_INFO(timedate) << "Set seconds showing to" << secondsShowing;
    return true;
}

QStringList TimedateFormat::getLongFormats()
{
    auto formats = QStringList{
        "%A, " + QString(tr("%b %d %Y")),
        QString(tr("%b %d %Y")) + ", %A",
        tr("%b %d %Y"),
        "%Y/%m/%d, %A",
        "%A, %Y/%m/%d",
        "%Y-%m-%d, %A",
        "%A, %Y-%m-%d",
        "%Y.%m.%d, %A",
        "%A, %Y.%m.%d"};
    return formats;
}

QStringList TimedateFormat::getShortFormats()
{
    auto formats = QStringList{
        "%Y/%m/%d",
        "%Y.%m.%d",
        "%Y-%m-%d"};
    return formats;
}

}  // namespace Kiran