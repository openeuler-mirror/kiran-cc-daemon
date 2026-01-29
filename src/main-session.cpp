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

/* 控制中心会话后端一般是会话管理拉起的第一个程序，一般情况下，需要控制中心会话后端进行初始化完毕后再启动其他会话程序，
   这里的逻辑大致如此：
      1. 定义QApplication app变量，此时会调用SmcOpenConnection函数打开xsmp连接（可自行gdb断点查看），会话管理后端会收到
         register_client.callback函数回调，并保存clientId和连接句柄SmsConn的对应关系。
      2. 继续调用Kiran::SessionPluginManager::globalInit初始化所有插件。
      3. 调用app.exec函数，启动事件循环，此时控制中心后端会调用sm_setProperty(QString::fromLatin1(SmProgram), argument0)，
         向会话管理发送自己的程序名，此时会话管理可将clientId和实际的app关联起来，知道控制中心后端已经完成初始化，此时会话管理的
         Initialization阶段完成。
      4. 会话管理程序开始进入下一个阶段，开始启动会话中其他程序。
*/

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

    auto retval = app.exec();

    Kiran::SessionPluginManager::globalDeinit();
    Kiran::InputBackend::globalDeinit();
    Kiran::OSDWindow::globalDeinit();

    return retval;
}
