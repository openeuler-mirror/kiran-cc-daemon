# kiran-cc-daemon

The project consists of two backend services: the system backend (`kiran-system-daemon`) and the session backend (`kiran-session-daemon`).

## Build and Installation

```bash
# yum install cmake libxml++-devel glibmm24-devel glib2-devel gtkmm30-devel systemd-devel libselinux-devel gettext gcc-c++ intltool polkit dbus-daemon libX11-devel xerces-c-devel xsd fontconfig-devel jsoncpp-devel zlog-devel gdbus-codegen-glibmm fmt-devel gtest-devel upower-devel libnotify-devel pulseaudio-libs-devel
# mkdir build
# cd build && cmake -DCMAKE_INSTALL_PREFIX=/usr ..
# make
# make install
```

## Execution

These services start automatically upon system boot or when their respective D-Bus interfaces are invoked.

Alternatively, you can start them manually:

### kiran-system-daemon

```bash
systemctl start kiran-system-daemon.service
```

### kiran-session-daemon

```bash
kiran-session-daemon &
```
