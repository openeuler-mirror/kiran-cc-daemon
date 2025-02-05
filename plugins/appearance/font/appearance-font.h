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

#include "appearance-i.h"
#include "lib/base/base.h"

class QGSettings;

namespace Kiran
{
class AppearanceFont : public QObject
{
    Q_OBJECT

public:
    AppearanceFont(QObject *parent = nullptr);
    virtual ~AppearanceFont(){};

    void init();

    QString getFont(AppearanceFontType type);
    bool setFont(AppearanceFontType type, const QString &font);
    bool resetFont(AppearanceFontType type);

Q_SIGNALS:
    void fontChanged(AppearanceFontType type, const QString &font);

private:
    QString fontEnum2Str(AppearanceFontType type);
    void notifyFontChanged(const QString &key);

private:
    QGSettings *m_xsettingsSettings;
    QGSettings *m_interfaceSettings;
    QGSettings *m_marcoSettings;
    QGSettings *m_cajaSettings;
};
}  // namespace  Kiran
