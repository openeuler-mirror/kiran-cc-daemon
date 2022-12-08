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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <giomm.h>

namespace Kiran
{
class MiscUtils
{
public:
    MiscUtils();
    virtual ~MiscUtils(){};

    static Glib::OptionEntry create_option_entry(const char &short_name,
                                                 const Glib::ustring &long_name,
                                                 const Glib::ustring &description,
                                                 const Glib::ustring &arg_description = Glib::ustring(),
                                                 int32_t flags = 0);

    static Glib::OptionEntry create_option_entry(const Glib::ustring &long_name,
                                                 const Glib::ustring &description,
                                                 const Glib::ustring &arg_description = Glib::ustring(),
                                                 int32_t flags = 0);

};  // namespace KS

}  // namespace Kiran
