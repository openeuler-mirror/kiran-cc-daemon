/**
 * Copyright (c) 2026 ~ 2027 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * 
 * Author:     gaobo <gaobo@kylinsec.com.cn>
 */

#pragma once

#include <iostream>

#include <gtest/gtest.h>

// GTEST_SKIP() 自 gtest 1.10 起提供；更早版本退化为打印原因并返回（该用例计为通过）
#ifdef GTEST_SKIP
#define KIRAN_TEST_SKIP(msg) GTEST_SKIP() << msg
#else
#define KIRAN_TEST_SKIP(msg)                              \
    do                                                    \
    {                                                     \
        std::cerr << "[  SKIPPED ] " << msg << std::endl; \
        return;                                           \
    } while (0)
#endif
