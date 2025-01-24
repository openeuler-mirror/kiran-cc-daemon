/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
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

class KActionCollection;
class QGSettings;

namespace Kiran
{
class KeysComponent : public QObject
{
    Q_OBJECT

public:
    explicit KeysComponent(const QString &componentName, const QString &displayName);

public:
    virtual void init() = 0;
    virtual void triggerShortCut(const QString &name) = 0;

protected:
    bool registerShortCut(const QKeySequence &key, const QString &name, const QString &displayName);

public:
    KeysComponent();

protected:
    QGSettings *m_settings;

private:
    KActionCollection *m_actionCollection;
};
}  // namespace Kiran