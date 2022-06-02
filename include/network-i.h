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

#ifdef __cplusplus
extern "C"
{
#endif

#define NETWORK_PROXY_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.Network.Proxy"
#define NETWORK_PROXY_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Network/Proxy"
#define NETWORK_PROXY_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SessionDaemon.Network.Proxy"

/*
手动代理模式下JSON字符串格式如下
{
    "http": {
        "host": "127.0.0.1",
        "port": 8080,
        "enable_auth": false,
        "auth_password": "123456",
        "auth_user": "username"
    },
    "https": {
        "host": "127.0.0.1",
        "port": 8080
    },
    "ftp": {
        "host": "127.0.0.1",
        "port": 8080
    },
    "socks": {
        "host": "127.0.0.1",
        "port": 8080
    }
}
*/
#define NETWORK_PROXY_MJK_ENABLE_HTTP_AUTH "enable_auth"
#define NETWORK_PROXY_MJK_HTTP_AUTH_USER "auth_user"
#define NETWORK_PROXY_MJK_HTTP_AUTH_PASSWD "auth_password"

#define NETWORK_PROXY_MJK_HTTP "http"
#define NETWORK_PROXY_MJK_HTTPS "https"
#define NETWORK_PROXY_MJK_FTP "ftp"
#define NETWORK_PROXY_MJK_SOCKS "socks"

#define NETWORK_PROXY_MJK_HOST "host"
#define NETWORK_PROXY_MJK_PORT "port"

    /*如果设置为NETWORK_PROXY_MODE_NONE，则不应该使用网络代理；
      如果设置为NETWORK_PROXY_MODE_MANUAL，则需要手动配置代理参数，代理类型包括http、https、ftp和socket；
      如果设置为NETWORK_PROXY_MODE_AUTO，则代理信息通过设置的URL来获取。*/
    enum NetworkProxyMode
    {
        // 无
        NETWORK_PROXY_MODE_NONE,
        // 手动
        NETWORK_PROXY_MODE_MANUAL,
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