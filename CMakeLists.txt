cmake_minimum_required(VERSION 3.2...${CMAKE_VERSION})

project(kiran-cc-daemon VERSION 2.7)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(KSD REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets DBus LinguistTools Test)

pkg_search_module(KLOG_QT5 REQUIRED klog-qt5)
pkg_search_module(FMT REQUIRED fmt)
pkg_search_module(CRYPTOPP REQUIRED cryptopp)
pkg_search_module(GLIB2 REQUIRED glib-2.0)
find_package(KF5Config)

# session
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
pkg_search_module(LIBXML2 REQUIRED libxml++-2.6)
pkg_search_module(LIBNOTIFY REQUIRED libnotify)
pkg_search_module(XCB REQUIRED xcb)
pkg_search_module(XCB_IMAGE REQUIRED xcb-image)
pkg_search_module(XCB_XINPUT REQUIRED xcb-xinput)
pkg_search_module(XCB_RANDR REQUIRED xcb-randr)
pkg_search_module(XCB_DPMS REQUIRED xcb-dpms)
pkg_search_module(FONTCONFIG REQUIRED fontconfig)
pkg_search_module(LIBPULSE REQUIRED libpulse)
pkg_search_module(LIBPULSE_MAINLOOP REQUIRED libpulse-mainloop-glib)
pkg_search_module(XERCES REQUIRED xerces-c)
pkg_search_module(Qt5XDG REQUIRED Qt5Xdg)
find_package(KF5GlobalAccel REQUIRED)
find_package(KF5XmlGui REQUIRED)
find_package(KF5Screen REQUIRED)
find_package(KF5GuiAddons REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(Qt5 COMPONENTS X11Extras)
find_package(KF5Service)
find_package(KF5KIO)

# system
pkg_search_module(LIBGTOP REQUIRED libgtop-2.0)
pkg_search_module(GUDEV REQUIRED gudev-1.0)
pkg_search_module(LIBXCRYPT REQUIRED libxcrypt)
pkg_search_module(SYSTEMD REQUIRED systemd)
pkg_search_module(SELINUX REQUIRED libselinux)

configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)

add_subdirectory(po)
add_subdirectory(data)
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(plugins)
add_subdirectory(test)
