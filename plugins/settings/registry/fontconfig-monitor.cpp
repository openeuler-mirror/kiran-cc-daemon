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

#include "fontconfig-monitor.h"
#include <fontconfig/fontconfig.h>
#include <QFileSystemWatcher>
#include <QTimer>
#include "lib/base/base.h"

namespace Kiran
{
#define FONTCONFIG_UPDATE_TIMEOUT_SECONDS 2

FontconfigMonitor::FontconfigMonitor(QObject *parent) : QObject(parent)
{
    m_filesWatcher = new QFileSystemWatcher(this);
    m_timer = new QTimer(this);
}

void FontconfigMonitor::init()
{
    m_timer->setInterval(1000);

    FcInit();
    loadFilesMonitors();

    connect(m_filesWatcher, SIGNAL(fileChanged(const QString &)), m_timer, SLOT(start()));
    connect(m_filesWatcher, SIGNAL(directoryChanged(const QString &)), m_timer, SLOT(start()));
    connect(m_timer, &QTimer::timeout, this, &FontconfigMonitor::updateFontConfig);
}

void FontconfigMonitor::loadFilesMonitors()
{
    auto oldPaths = m_filesWatcher->files();
    oldPaths.append(m_filesWatcher->directories());

    if (oldPaths.size() > 0)
    {
        m_filesWatcher->removePaths(oldPaths);
    }

    addFilesMonitors(FcConfigGetConfigFiles(NULL));
    addFilesMonitors(FcConfigGetFontDirs(NULL));
}

void FontconfigMonitor::addFilesMonitors(FcStrList *files)
{
    const char *str;
    while ((str = reinterpret_cast<const char *>(FcStrListNext(files))))
    {
        m_filesWatcher->addPath(str);
    }
    FcStrListDone(files);
}

void FontconfigMonitor::updateFontConfig()
{
    KLOG_INFO(settings) << "Font files is changes. so reload fontconfig";

    m_timer->stop();
    if (!FcConfigUptoDate(NULL) && FcInitReinitialize())
    {
        loadFilesMonitors();
        Q_EMIT timestampChanged();
    }
}
}  // namespace Kiran
