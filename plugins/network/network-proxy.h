/**
 * @file          /kiran-cc-daemon/plugins/network/network-proxy.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

namespace Kiran
{
class NetworkProxy
{
public:
    NetworkProxy(){};
    virtual ~NetworkProxy(){};

    static NetworkProxy *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

private:
    void init();

private:
    static NetworkProxy *instance_;
};
}  // namespace Kiran
