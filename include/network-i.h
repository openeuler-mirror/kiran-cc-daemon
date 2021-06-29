/**
 * @file          /kiran-cc-daemon/include/network-i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define NETWORK_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.Network"
#define NETWORK_PROXY_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Network/Proxy"
#define NETWORK_PROXY_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SessionDaemon.Network.Proxy"

    /*如果设置为NETWORK_PROXY_MODE_NONE，则不应该使用网络代理；
      如果设置为NETWORK_PROXY_MODE_MANUUAL，则需要手动配置代理参数，代理类型包括http、https、ftp和socket；
      如果设置为NETWORK_PROXY_MODE_AUTO，则代理信息通过设置的URL来获取。*/
    enum NetworkProxyMode
    {
        // 无
        NETWORK_PROXY_MODE_NONE,
        // 手动
        NETWORK_PROXY_MODE_MANUUAL,
        // 自动
        NETWORK_PROXY_MODE_AUTO,
        NETWORK_PROXY_MODE_LAST
    };

    enum NetworkProxyType
    {
        // HTTP
        NETWORK_PROXY_TYPE_HTTP,
        // HTTPS
        NETWORK_PROXY_TYPE_HTTPS,
        // FTP
        NETWORK_PROXY_TYPE_FTP,
        // SOCKET
        NETWORK_PROXY_TYPE_SOCKET,
        NETWORK_PROXY_TYPE_LAST,
    };

#ifdef __cplusplus
}
#endif