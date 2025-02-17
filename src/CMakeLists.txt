cmake_minimum_required(VERSION 3.0)

qt5_add_dbus_adaptor(
  DBUS_SRCS ${CMAKE_SOURCE_DIR}/src/com.kylinsec.Kiran.CCDaemon.Plugins.xml
  ${CMAKE_SOURCE_DIR}/src/plugin-manager.h Kiran::PluginManager
  plugin_manager_adaptor PluginManagerAdaptor)

# system daemon
set(TARGET_SYSTEM_DAEMON kiran-system-daemon)

add_executable(${TARGET_SYSTEM_DAEMON} ${DBUS_SRCS} main-system.cpp
                                       plugin-manager.cpp)

target_include_directories(
  ${TARGET_SYSTEM_DAEMON}
  PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR}/include ${KLOG_QT5_INCLUDE_DIRS}
          ${GSETTINGS_QT_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_SYSTEM_DAEMON}
  PRIVATE ${KLOG_QT5_LIBRARIES} ${GSETTINGS_QT_LIBRARIES} kbase Qt5::Core
          Qt5::DBus)

install(TARGETS ${TARGET_SYSTEM_DAEMON}
        DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

# session daemon

set(TARGET_SESSION_DAEMON kiran-session-daemon)

add_executable(${TARGET_SESSION_DAEMON} ${DBUS_SRCS} main-session.cpp
                                        plugin-manager.cpp session-guarder.cpp)

target_include_directories(
  ${TARGET_SESSION_DAEMON} PRIVATE ${PROJECT_SOURCE_DIR}
                                   ${GSETTINGS_QT_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_SESSION_DAEMON} PRIVATE ${GSETTINGS_QT_LIBRARIES} kbase kosdwindow
                                   kinput Qt5::DBus Qt5::Widgets)

install(TARGETS ${TARGET_SESSION_DAEMON}
        DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
