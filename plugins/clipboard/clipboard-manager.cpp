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

class ClipboardLockGuard
{
    Q_DISABLE_COPY(ClipboardLockGuard)
public:
    ClipboardLockGuard(int &count) : m_count(count)
    {
        ++m_count;
    }

    virtual ~ClipboardLockGuard()
    {
        --m_count;
    }

private:
    int &m_count;
};

ClipboardManager *ClipboardManager::m_instance = nullptr;
ClipboardManager::ClipboardManager()
{
    m_clipboard = KSystemClipboard::instance();

    for (int i = 0; i < QClipboard::FindBuffer; i++)
    {
        m_clipboardDatas[i] = QSharedPointer<ClipboardData>();
        m_clipboardLock[i] = 0;
    }

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
    /* 存在Clipboard和Selection两种情况：
     * Clipboard：剪切版中的内容，一般由快捷键ctrl+c触发或者通过选中文字后右键复制触发
     * Selection：鼠标选中的文字内容
     *
     * 不处理QClipboard::Selection情况见下面的解释
     */
    if (mode >= QClipboard::FindBuffer || mode == QClipboard::Selection)
    {
        return;
    }

    // 避免信号递归调用
    if (m_clipboardLock[mode])
    {
        return;
    }
    ClipboardLockGuard lockGuard(m_clipboardLock[mode]);

    /* 在X11协议规范中，CLIPBOARD_MANAGER对应的进程用于处理当CLIPBOARD对应进程退出时接管CLIPBOARD，接管前会从CLIPBOAD
       对应的进程中获取剪切版内容，然后成为CLIPBOARD的owner，这样确保了剪切版数据不会丢失。

       而kde的klipper实现中并没有使用CLIPBOARD_MANAGER（可能是为了兼容wayland实现？），使用了一种取巧的方法：每当有新的
       数据被选中、复制和取消时，会收到XCB_XFIXES_SELECTION_NOTIFY事件。
           1）如果是取消，则说明剪切版对应进程被关闭，此时需要接管剪切版，并将上一次复制或选中的数据拷贝到剪切版中。
           2）如果是选中或复制，则记录最新的数据作为历史数据。

       这里也采用klipper的实现方式：当剪切版数据为空时，则将上一次选中或复制的数据拷贝到剪切版中；否则将最新数据记录到
       m_clipboardDatas中。*/
    auto mimeData = m_clipboard->mimeData(mode);

    /* 当mode为QClipboard::Selection时，mimeData->formats()为空不能说明selection owner的进程退出了。所以覆盖QClipboard::Selection的内容不是很合理。
     * 如果一定要做，应该要通过XGetSelectionOwnwer判断是否为空，因为这个功能不实现也影响不大，所以这里不打算再引入X11的代码了。*/
    if (mimeData == nullptr || mimeData->formats().isEmpty())
    {
        if (m_clipboardDatas[mode])
        {
            auto newMimeData = m_clipboardDatas[mode]->mimeData();
            /*
             * 通过ssh -X到其他机器运行gtk程序时，选中文本后再取消文本选中，会触发QXcbClipboard::setMimeData: Cannot set X11 selection owner报错，
             * 所以这里不处理为QClipboard::Selection情况，猜测原因是取消文本选中后，QClipboard::Selection对应的owner不为空，导致无法设置新的selection owner
             *
             * uniface代码好像有问题，打开uniface虚拟机后，调用下面函数可能会导致触发QXcbClipboard: SelectionRequest too old告警，
             * uniface发送过来的请求中，req->time时间是错误的。
             */
            m_clipboard->setMimeData(newMimeData, mode);
        }
    }
    else
    {
        m_clipboardDatas[mode] = ClipboardData::createClipboardData(mimeData);
    }
}

}  // namespace Kiran