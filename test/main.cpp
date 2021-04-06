/**
 * @file          /kiran-cc-daemon/test/main.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include <giomm.h>
#include <gtest/gtest.h>
#include <zlog_ex.h>
#include "lib/base/base.h"

int main(int argc, char **argv)
{
    dzlog_init_ex(NULL, "kylinsec-system", "kiran-cc-daemon", "kiran-cc-test");

    Gio::init();
    Kiran::Log::global_init();

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
