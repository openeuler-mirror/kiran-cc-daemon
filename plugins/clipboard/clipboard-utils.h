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
 * 目标类型数据存储结构
 */
struct TargetData
{
    Atom target;
    Atom type;
    int format;
    unsigned long length;
    unsigned char *data;

    TargetData()
    {
        target = None;
        type = None;
        format = 0;
        length = 0;
        data = NULL;
    }

    ~TargetData()
    {
        // 销毁时释放内部数据
        if (data)
        {
            delete data;
            data = NULL;
        }
    }
};

/**
 * 转换类型数据存储结构
 */
struct IncrConversion
{
    Window requestor;
    Atom target;
    Atom property;
    // 数据偏移 为-1时表示非增量数据类型; 为>=0时表示增量INCR数据类型
    int offset;
    // 目标数据这里仅使用变量，数据内存由外部申请并由外部释放
    TargetData *data;

    IncrConversion()
    {
        requestor = None;
        target = None;
        property = None;
        offset = -1;
        data = NULL;
    }
};

/**
 * 窗口类型数据
 */
struct WindowPropRetStruct
{
    Atom type;
    int format;
    unsigned long nitems;
    unsigned long remaining;
    unsigned char *data;

    WindowPropRetStruct()
    {
        type = None;
        format = 0;
        nitems = 0;
        remaining = 0;
        data = NULL;
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

/**
 * 查找存储内容目标
 */
class FindContentTarget
{
public:
    FindContentTarget(Atom target) : target_(target) {}

    bool operator()(TargetData *targetdata)
    {
        if (targetdata->target == target_)
        {
            return true;
        }

        return false;
    }

private:
    Atom target_;
};

/**
 * 查找存储内容类型
 */
class FindContentType
{
public:
    FindContentType(Atom type) : type_(type) {}

    bool operator()(TargetData *tdata)
    {
        if (tdata->type == type_)
        {
            return true;
        }

        return false;
    }

private:
    Atom type_;
};

/**
 * 查找转换数据请求窗口
 */
class FindConversionRequestor
{
public:
    FindConversionRequestor(XEvent *xev) : xev_(xev) {}

    bool operator()(IncrConversion *rdata)
    {
        if (rdata->requestor == xev_->xproperty.window &&
            rdata->property == xev_->xproperty.atom)
        {
            return true;
        }

        return false;
    }

private:
    XEvent *xev_;
};

void init_atoms(Display *display);

void init_selection_max_size(Display *display);

int bytes_per_item(int format);

bool is_valid_target_in_save_targets(Atom target);

Time get_server_time(Display *display,
                     Window window);

/**
 * @brief 获取窗口属性参数
 * @param[in] {display} 
 * @param[in] {window} 窗口
 * @param[in] {property} 属性
 * @param[in] {is_delete} 删除标识
 * @param[in] {req_type} 请求类型
 * @param[out] {prop_ret} 窗口属性结构
 * @return 如果获取成功则返回true，否则返回false。
 */
bool get_window_property_return_struct(Display *display,
                                       Window window,
                                       Atom property,
                                       Bool is_delete,
                                       Atom req_type,
                                       WindowPropRetStruct *prop_ret);