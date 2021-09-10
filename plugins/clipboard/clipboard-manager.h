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

#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <list>
#include "plugins/clipboard/clipboard-utils.h"

namespace Kiran
{
/**
 * 窗口事件筛选项变更
 */
enum FilterChangeType
{
    FILTER_CHANGE_ADD = 0,
    FILTER_CHANGE_REMOVE = 1
};

class ClipboardManager
{
public:
    ClipboardManager();
    ~ClipboardManager(){};

public:
    static ClipboardManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit();

private:
    static GdkFilterReturn event_filter(GdkXEvent *xevent,
                                        GdkEvent *event,
                                        ClipboardManager *manager);

private:
    void init();

    void deinit();

    // 清空存储内容
    void clear_contents();

    // 清空转换数据
    void clear_conversions();

    // 发送客户端消息事件
    void send_client_message();

    // 保存目标数据内容
    void save_target_data(TargetData *tdata);

    // 转换剪切板目标数据
    void convert_clipboard_target(IncrConversion *rdata);

    // 收集转换数据
    void collect_incremental(IncrConversion *rdata);

    // 更改窗口事件筛选
    void change_window_filter(Window window, FilterChangeType type);

    // 处理事件
    bool process_event(XEvent *xev);

    // 处理selection为剪切板管理的SelectionRequest事件
    void process_selectionrequestor_clipboard_manager(XEvent *xev);

    // 处理selection为剪切板的SelectionRequest事件
    void process_selectionrequestor_clipboard(XEvent *xev);

    // 处理selection为剪切板的SelectionNotify事件
    void process_selectionnotify_clipboard(XEvent *xev);

    // 保存所有目标条目
    void save_targets(Atom *targets, unsigned long nitems);

    // 接收增量数据
    bool receive_incrementally(XEvent *xev);

    // 发送增量数据
    bool send_incrementally(XEvent *xev);

    // 响应管理服务保存目标事件请求
    void response_manager_save_targets(bool success);

    // 响应Selection请求
    void response_selection_request(XEvent *xev, bool success);

    // 处理保存目标数据的Selection请求
    void selection_request_save_targets(XEvent *xev);

    // 处理多类目标的Selection通知
    void selection_notify_clipboard_multiple(XEvent *xev);

    // 处理多重目标的Selection请求
    void selection_request_clipboard_multiple(XEvent *xev);

    // 处理单个目标的Selection请求
    void selection_request_clipboard_single(XEvent *xev);

private:
    static ClipboardManager *instance_;

    Display *display_;
    Window window_;
    Time timestamp_;

    Window requestor_;
    Atom property_;
    Time time_;

    std::list<TargetData *> contents_;
    std::list<IncrConversion *> conversions_;
};

}  // namespace Kiran
