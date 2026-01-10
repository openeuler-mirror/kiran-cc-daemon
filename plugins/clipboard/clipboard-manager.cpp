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
#include <xcb/xcb.h>
#include <KSystemClipboard>
#include <QGuiApplication>
#include <QMimeData>
#include "clipboard-data.h"
#include "lib/base/base.h"
#include "lib/xcb/xcb-connection.h"

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
ClipboardManager::ClipboardManager() : m_lastClipboardOwner(XCB_ATOM_NONE)
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

xcb_atom_t ClipboardManager::getClipboardOwner()
{
    if (qGuiApp->platformName() != "xcb")
    {
        return XCB_ATOM_NONE;
    }

    // 获取xcb_connection
    auto xcbConnection = XcbConnection::getDefault();
    auto clipboardAtomReply = XCB_REPLY(xcb_intern_atom, xcbConnection->getConnection(), false, strlen("CLIPBOARD"), "CLIPBOARD");

    if (!clipboardAtomReply || clipboardAtomReply->atom == XCB_ATOM_NONE)
    {
        return XCB_ATOM_NONE;
    }

    auto getClipboardOwnerReply = XCB_REPLY(xcb_get_selection_owner, xcbConnection->getConnection(), clipboardAtomReply->atom);
    return getClipboardOwnerReply ? getClipboardOwnerReply->owner : XCB_ATOM_NONE;
}

bool ClipboardManager::lastIsUDAPClient()
{
    if (qGuiApp->platformName() != "xcb")
    {
        return false;
    }

    // 获取m_clipboardOwner对应的进程
    auto xcbConnection = XcbConnection::getDefault();
    auto wmNameAtomReply = XCB_REPLY(xcb_intern_atom, xcbConnection->getConnection(), false, strlen("WM_NAME"), "WM_NAME");
    RETURN_VAL_IF_TRUE(!wmNameAtomReply, false);

    auto ownerWMNameReply = XCB_REPLY_UNCHECKED(xcb_get_property,
                                                xcbConnection->getConnection(),
                                                false,
                                                m_lastClipboardOwner,
                                                wmNameAtomReply->atom,
                                                XCB_ATOM_STRING,
                                                0,
                                                1024);
    RETURN_VAL_IF_TRUE(!ownerWMNameReply, false);

    if (ownerWMNameReply && ownerWMNameReply->format == 8 && ownerWMNameReply->type == XCB_ATOM_STRING)
    {
        auto name = reinterpret_cast<const char *>(xcb_get_property_value(ownerWMNameReply.get()));
        auto wmName = QString::fromLatin1(name, xcb_get_property_value_length(ownerWMNameReply.get()));
        return wmName == "UDAP Client";
    }
    return false;
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
    auto currentClipboardOwner = getClipboardOwner();

    /* 当之前的剪切版owner退出时，则由当前插件来接管并复制上次的剪切版数据到剪切版中。
     *     1）如果是wayland协议，暂时通过剪切版内容是否为空来判断；
     *     2）如果是x11协议，则通过剪切版owner是否为空来判断。
     */
    if ((mimeData == nullptr || mimeData->formats().isEmpty()) && currentClipboardOwner == XCB_ATOM_NONE)
    {
        /* 这里是为了跟云桌面uniface进行兼容做的定制化处理，当在云桌面虚拟机中进行第二次复制时，hostos的udap-client进程
           会先取消拥有剪切版（原因未知），然后获取剪切版的owner权限并将新复制的数据拷贝到剪切版，如果当前插件收到取消剪切版
           owner事件时去接管剪切版owner，就有可能导致udap-client无法将数据复制到剪切版中，考虑如下异步场景：
              1）udap-client进程取消剪切版owner权限；
              2）udap-client进程获取剪切版owner权限；
              3）当前插件收到取消剪切版owner权限事件，将会获取剪切版owner权限；
              4）udap-client进程将剪切版数据复制到剪切版中；当此时udap-client已经不是剪切版owner了，所以数据复制失败。

           因此，当前插件收到取消剪切版owner权限事件且上一次剪切版owner为udap-client时，不去接管剪切版owner。

           关联单号 #111540
        */
        if (lastIsUDAPClient())
        {
            KLOG_DEBUG() << "Don't take over clipboard when last clipboard owner is udap-client";
            return;
        }

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

    m_lastClipboardOwner = currentClipboardOwner;
}

}  // namespace Kiran