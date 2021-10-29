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
 * Author:     meizhigang <meizhigang@kylinos.com.cn>
 */

#pragma once

#include "lib/base/base.h"

#include <gdk/gdkx.h>
#include "plugins/clipboard/clipboard-data.h"
#include "plugins/clipboard/clipboard-utils.h"
#include "plugins/clipboard/clipboard.h"

class ClipboardSave;
class ClipboardSend;

namespace Kiran
{
/**
 * 剪切板管理类 
 * 包含目标为SAVE_TARGETS的事件处理机制,存储窗口数据到管理服务中
 * 监听display相关窗口事件，并响应窗口请求
 */
class ClipboardManager
{
public:
    ClipboardManager();
    ~ClipboardManager();

    static ClipboardManager *get_instance() { return instance_; }

    static void global_init();

    static void global_deinit();

    static GdkFilterReturn event_filter(GdkXEvent *xevent,
                                        GdkEvent *event,
                                        ClipboardManager *manager);

private:
    void init();

    void deinit();

    // 清理剪切板存储请求者窗口
    void clear_requestor();

    // 清理剪切板所有者
    void clear_clipboard_owner();

    // 处理通信各类事件
    bool process_event(XEvent *xev);

    // 剪切板管理服务启动后 发送ClientMessage消息
    void send_client_message();

    // 处理 Selection 为 CLIPBOARD_MANAGER 相关的 SelectionRequest 消息
    void selection_request_clipboard_manager(XEvent *xev);

    // 处理 Target 为 SAVE_TARGETS 相关的 SelectionRequest 消息
    void selection_request_save_targets(XEvent *xev);

    // 保存target列表数据
    void save_targets(Atom *targets, unsigned long nitems);

    // 处理 SelectionNotify 消息
    void selection_notify(XEvent *xev);

    // 保存多类target窗口属性数据
    void save_multiple_property(XEvent *xev);

    // 响应 Selection：CLIPBOARD_MANAGER 并且Target：SAVE_TARGETS的消息
    void response_manager_save_targets(bool success);

    // 接收剪切板增量更新的数据内容
    bool receive_incrementally(XEvent *xev);

private:
    static ClipboardManager *instance_;

    Display *display_;
    // 剪切板窗口
    Window window_;
    // 剪切板管理服务事件戳
    Time timestamp_;

    // 剪切板存储请求者窗口
    Window requestor_;
    // 请求者属性
    Atom property_;
    // 请求者时间
    Time time_;

    // 剪切板数据管理类
    ClipboardData clipboard_data_;
    // 剪切板内容
    Clipboard clipboard_;
};
}  // namespace Kiran
