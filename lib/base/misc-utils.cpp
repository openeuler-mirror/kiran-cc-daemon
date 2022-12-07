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

#include "lib/base/misc-utils.h"
#include "lib/base/base.h"

namespace Kiran
{
MiscUtils::MiscUtils()
{
}

Glib::OptionEntry MiscUtils::create_option_entry(const char &short_name,
                                                 const Glib::ustring &long_name,
                                                 const Glib::ustring &description,
                                                 const Glib::ustring &arg_description,
                                                 int32_t flags)
{
    Glib::OptionEntry result;
    result.set_short_name(short_name);
    result.set_long_name(long_name);
    result.set_description(description);
    result.set_arg_description(arg_description);
    result.set_flags(flags);
    return result;
}

Glib::OptionEntry MiscUtils::create_option_entry(const Glib::ustring &long_name,
                                                 const Glib::ustring &description,
                                                 const Glib::ustring &arg_description,
                                                 int32_t flags)
{
    Glib::OptionEntry result;
    result.set_long_name(long_name);
    result.set_description(description);
    result.set_arg_description(arg_description);
    result.set_flags(flags);
    return result;
}
}  // namespace Kiran
