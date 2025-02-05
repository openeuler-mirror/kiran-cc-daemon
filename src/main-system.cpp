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

#include <qt5-log-i.h>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include "config.h"
#include "src/plugin-manager.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    if (klog_qt5_init(QString(), "kylinsec-system", PROJECT_NAME, QCoreApplication::applicationName()) < 0)
    {
        fprintf(stderr, "Failed to init kiran-log.");
    }

    Kiran::SystemPluginManager::globalInit(KCD_SYSTEM_PLUGIN_DIR);

    auto retval = app.exec();

    Kiran::SystemPluginManager::globalDeinit();

    return retval;
}
