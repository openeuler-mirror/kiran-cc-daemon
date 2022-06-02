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

#include <map>
#include "lib/base/base.h"

#include <X11/Xlib.h>
#include "plugins/clipboard/clipboard-utils.h"

namespace Kiran
{
/**
 * 目标类型数据存储结构
 */
struct TargetData
{
    // 属性目标标识
    Atom target;
    // 属性类型
    Atom type;
    // 属性数据格式
    int format;
    // 数据长度
    unsigned long length;
    // 数据内容
    unsigned char *data;

    TargetData()
    {
        target = None;
        type = None;
        format = 0;
        length = 0;
        data = nullptr;
    }

    ~TargetData()
    {
        if (data)
        {
            delete[] data;
            data = nullptr;
        }
    }
};

class ClipboardData
{
public:
    ClipboardData();
    virtual ~ClipboardData();

    void init();

    void deinit();

    // 清空剪切板内容
    void clear_contents();

    // 剪切板内容是否为空
    bool is_contents_empty();

    // 剪切板是否包含类型数据
    bool is_exist_type(Atom type);

    // 添加属性target类型
    void add_target_data(Atom target);

    // 获取剪切板内容taget列表数据
    std::vector<Atom> get_targets();

    // 获取目标数据内容
    std::shared_ptr<TargetData> get_target_data_by_target(Atom target);

    // 保存所有target类型属性数据
    void save_targets_data(Display *display, Window window);

    // 保存增量更新的target属性数据
    void save_incremental_target_data(std::shared_ptr<TargetData> tdata,
                                      const WindowPropertyGroup &prop_group);

private:
    // 保存剪切板目标数据
    void save_target_data(Display *display, Window window, Atom target);

private:
    // 格式：<target, TargetData>
    std::map<Atom, std::shared_ptr<TargetData>> contents_;
};
}  // namespace Kiran
