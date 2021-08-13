/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#include "plugins/timedate/timedate-format.h"

#include <glib/gi18n.h>

namespace Kiran
{
#define TIMEDATE_FORMAT_CONFIG_DIR "/etc/kiran-cc-daemon/system/timedate/"
#define TIMEDATE_FORMAT_CONFIG_FILENAME "timedate.conf"

#define TIMEDATE_FORMAT_GROUP_NAME "format"
#define TIMEDATE_FORMAT_KEY_DATE_LONG_FORMAT_INDEX "date_long_format_index"
#define TIMEDATE_FORMAT_KEY_DATE_SHORT_FORMAT_INDEX "date_short_format_index"
#define TIMEDATE_FORMAT_KEY_HOUR_FORMAT "hour_format"
#define TIMEDATE_FORMAT_KEY_SECONDS_SHOWING "seconds_showing"

TimedateFormat::TimedateFormat()
{
    this->format_config_path_ = Glib::build_filename(std::vector<std::string>{TIMEDATE_FORMAT_CONFIG_DIR, TIMEDATE_FORMAT_CONFIG_FILENAME});
}

TimedateFormat::~TimedateFormat()
{
}

void TimedateFormat::init()
{
    try
    {
        this->format_config_.load_from_file(this->format_config_path_, Glib::KEY_FILE_KEEP_COMMENTS);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return;
    }
}

int32_t TimedateFormat::get_date_long_format_index()
{
    KLOG_PROFILE("");

    // 获取不到则使用第一个
    int32_t long_format_index = 0;
    IGNORE_EXCEPTION(long_format_index = this->format_config_.get_integer(TIMEDATE_FORMAT_GROUP_NAME, TIMEDATE_FORMAT_KEY_DATE_LONG_FORMAT_INDEX));
    // 如果索引超出范围，则设置为第一个
    RETURN_VAL_IF_TRUE(long_format_index < 0 || long_format_index >= (int32_t)this->get_long_formats().size(), 0);
    return long_format_index;
}

int32_t TimedateFormat::get_date_short_format_index()
{
    KLOG_PROFILE("");

    int32_t short_format_index = 0;
    IGNORE_EXCEPTION(short_format_index = this->format_config_.get_integer(TIMEDATE_FORMAT_GROUP_NAME, TIMEDATE_FORMAT_KEY_DATE_SHORT_FORMAT_INDEX));
    // 如果索引超出范围，则设置为第一个
    RETURN_VAL_IF_TRUE(short_format_index < 0 || short_format_index >= (int32_t)this->get_short_formats().size(), 0);
    return short_format_index;
}

TimedateHourFormat TimedateFormat::get_hour_format()
{
    KLOG_PROFILE("");

    int32_t hour_format = 0;
    IGNORE_EXCEPTION(hour_format = this->format_config_.get_integer(TIMEDATE_FORMAT_GROUP_NAME, TIMEDATE_FORMAT_KEY_HOUR_FORMAT));
    // 如果索引超出范围，则设置为12小时制
    RETURN_VAL_IF_TRUE(hour_format < 0 || hour_format >= TimedateHourFormat::TIMEDATE_HOUSR_FORMAT_LAST,
                       TimedateHourFormat::TIMEDATE_HOUSR_FORMAT_12_HOURS);
    return TimedateHourFormat(hour_format);
}

bool TimedateFormat::get_seconds_showing()
{
    KLOG_PROFILE("");
    bool seconds_showing = false;
    IGNORE_EXCEPTION(seconds_showing = this->format_config_.get_boolean(TIMEDATE_FORMAT_GROUP_NAME, TIMEDATE_FORMAT_KEY_SECONDS_SHOWING));
    return seconds_showing;
}

bool TimedateFormat::set_date_long_format(int32_t index)
{
    KLOG_PROFILE("index: %d.", index);

    RETURN_VAL_IF_TRUE(index < 0 || index >= (int32_t)this->get_long_formats().size(), false);
    RETURN_VAL_IF_TRUE(index == this->get_date_long_format_index(), true);

    this->format_config_.set_integer(TIMEDATE_FORMAT_GROUP_NAME, TIMEDATE_FORMAT_KEY_DATE_LONG_FORMAT_INDEX, index);

    return this->save_to_config();
}

bool TimedateFormat::set_date_short_format(int32_t index)
{
    KLOG_PROFILE("index: %d.", index);

    RETURN_VAL_IF_TRUE(index < 0 || index >= (int32_t)this->get_short_formats().size(), false);
    RETURN_VAL_IF_TRUE(index == this->get_date_short_format_index(), true);

    this->format_config_.set_integer(TIMEDATE_FORMAT_GROUP_NAME, TIMEDATE_FORMAT_KEY_DATE_SHORT_FORMAT_INDEX, index);
    return this->save_to_config();
}

bool TimedateFormat::set_hour_format(TimedateHourFormat hour_format)
{
    KLOG_PROFILE("hour_format: %d.", hour_format);

    this->format_config_.set_integer(TIMEDATE_FORMAT_GROUP_NAME, TIMEDATE_FORMAT_KEY_HOUR_FORMAT, hour_format);
    return this->save_to_config();
}

bool TimedateFormat::set_seconds_showing(bool seconds_showing)
{
    KLOG_PROFILE("seconds_showing: %d.", seconds_showing);

    this->format_config_.set_integer(TIMEDATE_FORMAT_GROUP_NAME, TIMEDATE_FORMAT_KEY_SECONDS_SHOWING, seconds_showing);
    return this->save_to_config();
}

std::vector<std::string> TimedateFormat::get_long_formats()
{
    auto formats = std::vector<std::string>{
        "%A, " + std::string(_("%b %d %Y")),
        std::string(_("%b %d %Y")) + ", %A",
        _("%b %d %Y"),
        "%Y/%m/%d, %A",
        "%A, %Y/%m/%d",
        "%Y-%m-%d, %A",
        "%A, %Y-%m-%d",
        "%Y.%m.%d, %A",
        "%A, %Y.%m.%d"};
    return formats;
}

std::vector<std::string> TimedateFormat::get_short_formats()
{
    auto formats = std::vector<std::string>{
        "%Y/%m/%d",
        "%Y.%m.%d",
        "%Y-%m-%d"};
    return formats;
}

bool TimedateFormat::save_to_config()
{
    try
    {
        this->format_config_.save_to_file(format_config_path_);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return false;
    }
    return true;
}

}  // namespace Kiran