/**
 * Copyright (c) 2020 ~ 2026 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */

#pragma once

#include <QObject>

class QGSettings;

namespace Kiran
{
class RegistryKde : public QObject
{
    Q_OBJECT
public:
    RegistryKde(QObject *parent = nullptr);
    virtual ~RegistryKde(){};

    void init();

private:
    void sync2KdeSettings(const QString &key);
    void notifyKGlobalSettingsChange(int type, int arg);

private:
    QGSettings *m_settings;
    struct KConfigInfo
    {
        QString file;
        QString group;
        QString key;
    };
    const static QMap<QString, KConfigInfo> s_schema2KdeSchema;
};
}  // namespace Kiran