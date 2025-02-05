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
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#pragma once

#include <QClipboard>
#include <QObject>
#include <QSharedPointer>

class KSystemClipboard;

namespace Kiran
{

class ClipboardData;

class ClipboardManager : public QObject
{
    Q_OBJECT

public:
    ClipboardManager();

    static ClipboardManager *get_instance() { return m_instance; }

    static void globalInit();
    static void globalDeinit();

private:
    void init();
    void processClipboardChanged(QClipboard::Mode mode);

private:
    static ClipboardManager *m_instance;

    KSystemClipboard *m_clipboard;
    QSharedPointer<ClipboardData> m_clipboardDatas[QClipboard::FindBuffer];
};
}  // namespace Kiran
