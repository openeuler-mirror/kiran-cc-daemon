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
#include "plugins/clipboard/clipboard-data.h"
#include "plugins/clipboard/clipboard-utils.h"

namespace Kiran
{
/**
 * 增量转换数据偏移标志
 */
enum IncrOffsetType
{
    // 表示无增量数据
    INCR_OFFSET_NONE = -1,
    // 表示增量数据起始
    INCR_OFFSET_START = 0
};

/**
 * 转换类型数据存储结构
 */
struct IncrConversion
{
    // 请求窗口
    Window requestor;
    // 属性目标
    Atom target;
    // 属性内容
    Atom property;
    // 数据偏移 为-1时表示非增量数据类型
    int offset;
    // 目标数据这里仅使用变量，数据内存由外部申请并由外部释放
    std::shared_ptr<TargetData> data;

    IncrConversion()
    {
        requestor = None;
        target = None;
        property = None;
        offset = INCR_OFFSET_NONE;
    }
};

/**
 * Clipboard处理请求剪切板数据相关操作
 * 存储请求剪切板内容的请求者及请求类型，发送剪切板数据内容到对应请求者
 */
class Clipboard
{
public:
    Clipboard();
    virtual ~Clipboard();

    void init(Display* display,
              Window window,
              GdkFilterFunc filter_func,
              void* user_data,
              ClipboardData* clipboard_data);

    void deinit();

    // 发送增量更新的数据内容
    bool send_incrementally(XEvent* xev);

    // 处理 Selection 为 CLIPBOARD 相关的 SelectionRequest 消息
    void selection_request_clipboard(XEvent* xev);

private:
    // 处理多类目标属性请求消息
    void selection_request_clipboard_multiple(XEvent* xev);

    // 处理单个目标属性请求消息
    void selection_request_clipboard_single(XEvent* xev);

    // 转换剪切板target数据
    void convert_clipboard_target(std::shared_ptr<IncrConversion> rdata);

    // 转换剪切板TAGETS类型数据
    void convert_type_targets(std::shared_ptr<IncrConversion> rdata);

    // 转换剪切板TARGETS以外类型数据
    void convert_type_without_targets(std::shared_ptr<IncrConversion> rdata);

    // 采集增量更新数据
    void collect_incremental(std::shared_ptr<IncrConversion> rdata);

private:
    Display* display_;
    Window window_;
    GdkFilterFunc filter_func_;
    void* user_data_;
    ClipboardData* clipboard_data_;

    std::vector<std::shared_ptr<IncrConversion>> conversions_;
};
}  // namespace Kiran
