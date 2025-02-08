set(TARGET_NAME keybinding)

file(GLOB_RECURSE KEYBINDING_H_FILES ./*.h)
file(GLOB_RECURSE KEYBINDING_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  KEYBINDING_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.Keybinding.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/keybinding-manager.h Kiran::KeybindingManager)

set(KGLOBALACCEL_XML
    ${KGLOBALACCEL_DBUS_INTERFACES_DIR}/kf5_org.kde.KGlobalAccel.xml)
set_source_files_properties(
  ${KGLOBALACCEL_XML}
  PROPERTIES INCLUDE "kglobalaccel.h"
             NO_NAMESPACE TRUE
             CLASSNAME "KGlobalAccelInterface")
qt5_add_dbus_interface(KGLOBAL_ACCEL_SRCS ${KGLOBALACCEL_XML}
                       kglobalaccel_interface)

set(KGLOBALACCEL_COMPONENT_XML
    ${KGLOBALACCEL_DBUS_INTERFACES_DIR}/kf5_org.kde.kglobalaccel.Component.xml)
set_source_files_properties(
  ${KGLOBALACCEL_COMPONENT_XML}
  PROPERTIES INCLUDE "kglobalaccel.h"
             NO_NAMESPACE TRUE
             CLASSNAME "KGlobalAccelComponentInterface")
qt5_add_dbus_interface(KGLOBAL_ACCEL_SRCS ${KGLOBALACCEL_COMPONENT_XML}
                       kglobalaccel_component_interface)

set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/plugins/audio/com.kylinsec.Kiran.SessionDaemon.Audio.xml
  PROPERTIES CLASSNAME AudioProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  AUDIO_DBUS_SRCS
  ${PROJECT_SOURCE_DIR}/plugins/audio/com.kylinsec.Kiran.SessionDaemon.Audio.xml
  audio_dbus_proxy)

set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/plugins/audio/com.kylinsec.Kiran.SessionDaemon.Audio.Device.xml
  PROPERTIES CLASSNAME AudioDeviceProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  AUDIO_DEVICE_DBUS_SRCS
  ${PROJECT_SOURCE_DIR}/plugins/audio/com.kylinsec.Kiran.SessionDaemon.Audio.Device.xml
  audio_device_dbus_proxy)

set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/plugins/touchpad/com.kylinsec.Kiran.SessionDaemon.TouchPad.xml
  PROPERTIES CLASSNAME TouchpadProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  TOUCHPAD_DBUS_SRCS
  ${PROJECT_SOURCE_DIR}/plugins/touchpad/com.kylinsec.Kiran.SessionDaemon.TouchPad.xml
  touchpad_dbus_proxy)

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts"
)

execute_process(COMMAND lupdate-qt5 -no-obsolete ${KEYBINDING_H_FILES}
                        ${KEYBINDING_CPP_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

add_library(
  ${TARGET_NAME} SHARED
  ${KEYBINDING_H_FILES}
  ${KEYBINDING_CPP_FILES}
  ${KEYBINDING_SRCS}
  ${KGLOBAL_ACCEL_SRCS}
  ${AUDIO_DBUS_SRCS}
  ${AUDIO_DEVICE_DBUS_SRCS}
  ${TOUCHPAD_DBUS_SRCS}
  ${QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_BINARY_DIR}
          ${CMAKE_CURRENT_BINARY_DIR}
          ${GSETTINGS_QT_INCLUDE_DIRS}
          ${GLIB2_INCLUDE_DIRS}
          ${LIBXML2_INCLUDE_DIRS}
          ${Qt5XDG_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${XCB_LIBRARIES}
          ${LIBNOTIFY_LIBRARIES}
          ${GSETTINGS_QT_LIBRARIES}
          ${GLIB2_LIBRARIES}
          ${LIBXML2_LIBRARIES}
          ${Qt5XDG_LIBRARIES}
          kbase
          kxcb
          kosdwindow
          KF5::GlobalAccel
          KF5::XmlGui
          KF5::WindowSystem
          KF5::Service
          KF5::KIOCore)

install(FILES ${QM_FILES} DESTINATION ${KCD_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/session)
