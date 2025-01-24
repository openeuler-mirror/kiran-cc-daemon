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

#include <QObject>
#include "timedate-i.h"

class QSettings;

namespace Kiran
{
class TimedateFormat : public QObject
{
    Q_OBJECT

public:
    TimedateFormat(QObject *parent = nullptr);
    virtual ~TimedateFormat();

    void init();

    // 获取已设置的长格式索引，通过索引从长格式列表中查找格式字符串
    int32_t getDateLongFormatIndex();
    // 获取已设置的短格式索引，通过索引从短格式列表中查找格式字符串
    int32_t getDateShortFormatIndex();
    // 获取小时显示格式：12小时制和24小时制
    TimedateHourFormat getHourFormat();
    // 时间中是否显示秒
    bool getSecondsShowing();

    // 设置长格式
    bool setDateLongFormat(int32_t index);
    // 设置短格式
    bool setDateShortFormat(int32_t index);
    // 设置小时显示格式
    bool setHourFormat(TimedateHourFormat hour_format);
    // 设置是否显示秒
    bool setSecondsShowing(bool seconds_showing);

    // 获取可设置的长格式列表，格式字符串已根据语言环境进行翻译
    QStringList getLongFormats();
    // 获取可设置的短格式列表
    QStringList getShortFormats();

private:
    QSettings *m_settings;
};
}  // namespace Kiran