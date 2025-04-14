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

#include <execinfo.h>
#include <qt5-log-i.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include "config.h"
#include "lib/base/misc-utils.h"
#include "lib/input/input-backend.h"
#include "lib/osdwindow/osd-window.h"
#include "src/plugin-manager.h"
#include "src/session-guarder.h"

static void sessionEnd()
{
    Kiran::SessionPluginManager::getInstance()->deactivatePlugins();

    KLOG_INFO() << "All plugins are deactivated, process is exited normally.";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationVersion(PROJECT_VERSION);
    QApplication::setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    if (klog_qt5_init(QString(), "kylinsec-session", PROJECT_NAME, QCoreApplication::applicationName()) < 0)
    {
        fprintf(stderr, "Failed to init kiran-log.");
    }

    Kiran::MiscUtils::installTranslator(QString("%1-%2").arg(PROJECT_NAME).arg("kbase"));

    Kiran::OSDWindow::globalInit();
    Kiran::InputBackend::globalInit();
    Kiran::SessionPluginManager::globalInit(KCD_SESSION_PLUGIN_DIR);
    Kiran::SessionGuarder::globalInit();
    auto sessionGuarder = Kiran::SessionGuarder::getInstance();
    QObject::connect(sessionGuarder, &Kiran::SessionGuarder::sessionEnd, &sessionEnd);

    auto retval = app.exec();

    Kiran::SessionGuarder::globalDeinit();
    Kiran::SessionPluginManager::globalDeinit();
    Kiran::InputBackend::globalDeinit();
    Kiran::OSDWindow::globalDeinit();

    return retval;
}
