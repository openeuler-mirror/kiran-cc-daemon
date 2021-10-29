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
//
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>

namespace Kiran
{
extern Atom XA_NULL;
extern Atom XA_SAVE_TARGETS;
extern Atom XA_TARGETS;
extern Atom XA_TIMESTAMP;
extern Atom XA_ATOM_PAIR;
extern Atom XA_DELETE;
extern Atom XA_INCR;
extern Atom XA_MULTIPLE;
extern Atom XA_INSERT_PROPERTY;
extern Atom XA_INSERT_SELECTION;
extern Atom XA_MANAGER;
extern Atom XA_CLIPBOARD_MANAGER;
extern Atom XA_CLIPBOARD;

extern unsigned long SELECTION_MAX_SIZE;

/**
 * 窗口事件筛选项变更
 */
enum FilterChangeType
{
    FILTER_CHANGE_ADD = 0,
    FILTER_CHANGE_REMOVE = 1
};

/**
 * 窗口属性组结构
 */
struct WindowPropertyGroup
{
    Atom type;
    int format;
    unsigned long nitems;
    unsigned long remaining;
    unsigned char *data;

    WindowPropertyGroup()
    {
        type = None;
        format = 0;
        nitems = 0;
        remaining = 0;
        data = nullptr;
    }

    ~WindowPropertyGroup()
    {
        if (data)
        {
            XFree(data);
            data = nullptr;
        }
    }
};

/**
 * 时间戳
 */
typedef struct
{
    Window window;
    Atom timestamp_prop_atom;
} TimeStampInfo;

class ClipboardUtils
{
public:
    ClipboardUtils() {}
    ~ClipboardUtils() {}

    // 初始化Atoms
    static void init_atoms(Display *display);

    // 初始化selection允许最大数据长度SELECTION_MAX_SIZE
    static void init_selection_max_size(Display *display);

    // 每个条目数据格式字节数
    static int bytes_per_item(int format);

    // 判断当前目标是否有效
    static bool is_valid_target_in_save_targets(Atom target);

    // 获取服务器当前时间
    static Time get_server_time(Display *display, Window window);

    // 更改窗口消息筛选器
    static void change_window_filter(Window window,
                                     FilterChangeType type,
                                     GdkFilterFunc filter_func,
                                     void *user_data);

    // 响应SelectionRequest发送SelectionNotify
    static void response_selection_request(Display *display,
                                           XEvent *xev,
                                           bool success);

    /**
     * @brief 获取窗口属性参数
     * @param[in] {display}
     * @param[in] {window} 窗口
     * @param[in] {property} 属性
     * @param[in] {is_delete} 删除标识
     * @param[in] {req_type} 请求类型
     * @param[out] {prop_group} 窗口属性组结构
     * @return 如果获取成功则返回true，否则返回false。
    */
    static bool get_window_property_group(Display *display,
                                          Window window,
                                          Atom property,
                                          Bool is_delete,
                                          Atom req_type,
                                          WindowPropertyGroup *prop_group);
};

}  // namespace Kiran