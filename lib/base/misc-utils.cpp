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

#include "lib/base/misc-utils.h"
#include <QCoreApplication>
#include <QTranslator>
#include "config.h"
#include "log.h"

namespace Kiran
{
MiscUtils::MiscUtils()
{
}

QTranslator* MiscUtils::installTranslator(const QString& filename)
{
    auto translator = new QTranslator();
    if (!translator->load(QLocale(), filename, ".", KCD_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING() << "Load translation file" << filename << "failed.";
        delete translator;
        translator = nullptr;
    }
    else
    {
        QCoreApplication::installTranslator(translator);
    }
    return translator;
}

void MiscUtils::removeTranslator(QTranslator*& translator)
{
    if (translator)
    {
        QCoreApplication::removeTranslator(translator);
        delete translator;
        translator = nullptr;
    }
}

}  // namespace Kiran
