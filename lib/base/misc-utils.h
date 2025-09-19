/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
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

#include <QString>

class QTranslator;

namespace Kiran
{
class MiscUtils
{
public:
    MiscUtils();
    virtual ~MiscUtils() {};

    // 安装和卸载翻译
    static QTranslator* installTranslator(const QString& filename);
    static void removeTranslator(QTranslator*& translator);
};  // namespace KS

}  // namespace Kiran
