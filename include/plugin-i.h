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

#include <giomm.h>

namespace Kiran
{
class Plugin
{
public:
    virtual void activate() = 0;
    virtual void deactivate() = 0;
};

using NewPluginFun = void *(*)(void);
using DelPluginFun = void (*)(void *);

}  // namespace Kiran

#if defined(WIN32) || defined(_WIN64)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#define PLUGIN_EXPORT_FUNC_DEF(plugin_name)               \
    extern "C" DLLEXPORT void *new_plugin()               \
    {                                                     \
        return new Kiran::plugin_name();                  \
    }                                                     \
    extern "C" DLLEXPORT void delete_plugin(void *plugin) \
    {                                                     \
        delete (Kiran::plugin_name *)plugin;              \
    }
