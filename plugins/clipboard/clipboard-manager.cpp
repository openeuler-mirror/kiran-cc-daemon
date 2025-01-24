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

#include "clipboard-manager.h"
#include <KSystemClipboard>
#include <QMimeData>
#include "clipboard-data.h"

namespace Kiran
{
ClipboardManager *ClipboardManager::m_instance = nullptr;

ClipboardManager::ClipboardManager()
{
    m_clipboard = KSystemClipboard::instance();
    memset(m_clipboardDatas, 0, sizeof(m_clipboardDatas));

    connect(m_clipboard, &KSystemClipboard::changed, this, &ClipboardManager::processClipboardChanged);
}

void ClipboardManager::globalInit()
{
    m_instance = new ClipboardManager;
    m_instance->init();
}

void ClipboardManager::globalDeinit()
{
    delete m_instance;
}

void ClipboardManager::init()
{
}

void ClipboardManager::processClipboardChanged(QClipboard::Mode mode)
{
    /* 只处理Clipboard和Selection两种情况：
     * Clipboard：剪切版中的内容，一般由快捷键ctrl+c触发或者通过选中文字后右键复制触发
     * Selection：鼠标选中的文字内容
     */
    if (mode >= QClipboard::FindBuffer)
    {
        return;
    }

    /* 在X11协议规范中，CLIPBOARD_MANAGER对应的进程用于处理当CLIPBOARD对应进程退出时接管CLIPBOARD，接管前会从CLIPBOAD
       对应的进程中获取剪切版内容，然后成为CLIPBOARD的owner，这样确保了剪切版数据不会丢失。

       而kde的klipper实现中并没有使用CLIPBOARD_MANAGER（可能是为了兼容wayland实现？），使用了一种取巧的方法：每当有新的
       数据被选中、复制和取消时，会收到XCB_XFIXES_SELECTION_NOTIFY事件。
           1）如果是取消，则说明剪切版对应进程被关闭，此时需要接管剪切版，并将上一次复制或选中的数据拷贝到剪切版中。
           2）如果是选中或复制，则记录最新的数据作为历史数据。

       这里也采用klipper的实现方式：当剪切版数据为空时，则将上一次选中或复制的数据拷贝到剪切版中；否则将最新数据记录到
       m_clipboardDatas中。*/
    auto mimeData = m_clipboard->mimeData(mode);
    if (mimeData == nullptr || mimeData->formats().isEmpty())
    {
        if (m_clipboardDatas[mode])
        {
            auto newMimeData = m_clipboardDatas[mode]->mimeData();
            m_clipboard->setMimeData(newMimeData, mode);
        }
    }
    else
    {
        m_clipboardDatas[mode] = ClipboardData::createClipboardData(mimeData);
    }
}

}  // namespace Kiran