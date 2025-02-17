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
 * Author:     meizhigang <meizhigang@kylinsec.com.cn>
 */

#pragma once

#include <QLabel>

class QTimer;

namespace Kiran
{
class OSDWindow : public QLabel
{
    Q_OBJECT

public:
    OSDWindow();
    virtual ~OSDWindow(){};
    static OSDWindow* getInstance() { return m_instance; };
    static void globalInit();
    static void globalDeinit() { delete m_instance; };

    void showIcon(const QString& iconName);

private:
    void init();
    void hideIcon();

private:
    static OSDWindow* m_instance;
    QString m_iconName;
    QTimer* m_timer;
};

}  // namespace  Kiran