/**
 * @file          /kiran-cc-daemon/plugins/network/network-proxy.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/network/network-proxy.h"

namespace Kiran
{
NetworkProxy* NetworkProxy::instance_ = nullptr;
void NetworkProxy::global_init()
{
    instance_ = new NetworkProxy();
    instance_->init();
}

void NetworkProxy::init()
{
}
}  // namespace Kiran
