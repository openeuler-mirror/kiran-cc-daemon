# 控制中心后端
项目包含系统后端(kiran-system-daemon)和会话后端(kiran-session-daemon)两个服务。

## 编译安装
```
# yum install cmake libxml++-devel glibmm24-devel glib2-devel gtkmm30-devel python3-setuptools systemd-devel libselinux-devel gettext gcc-c++ python3-jinja2 intltool polkit dbus-daemon libX11-devel xerces-c-devel xsd fontconfig-devel jsoncpp-devel
# mkdir build
# cd build && cmake -DCMAKE_INSTALL_PREFIX=/usr ..
# make
# make install
```

## 运行
系统启动后服务会自动启动，调用DBUS接口时服务也会自动启动。
也可以通过手动方式启动：
### kiran-system-daemon
```
systemctl start kiran-system-daemon.service
```
### kiran-session-daemon
```
kiran-session-daemon &
```