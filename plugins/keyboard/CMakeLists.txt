set(TARGET_NAME keyboard)

file(GLOB_RECURSE KEYBOARD_H_FILES ./*.h)
file(GLOB_RECURSE KEYBOARD_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  KEYBOARD_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.Keyboard.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/keyboard-manager.h Kiran::KeyboardManager)

add_library(${TARGET_NAME} SHARED ${KEYBOARD_H_FILES} ${KEYBOARD_CPP_FILES}
                                  ${KEYBOARD_SRCS})

pkg_get_variable(XKB_DATA_ROOT_DIR xorg-macros datarootdir)
if(NOT XKB_DATA_ROOT_DIR)
  set(XKB_DATA_ROOT_DIR "/usr/share")
endif()
set(XKB_BASE "${XKB_DATA_ROOT_DIR}/X11/xkb")

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}
          ${GSETTINGS_QT_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIRS})

target_compile_definitions(${TARGET_NAME} PRIVATE -DKCC_XKB_BASE="${XKB_BASE}")

pkg_get_variable(ISO_CODES_PREFIX iso-codes prefix)

if(NOT ISO_CODES_PREFIX)
  set(ISO_CODES_PREFIX "/usr")
endif()

target_compile_definitions(${TARGET_NAME}
                           PRIVATE -DKCC_ISO_CODES_PREFIX="${ISO_CODES_PREFIX}")

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${LIBXML2_LIBRARIES} ${GSETTINGS_QT_LIBRARIES} kbase osdwindow
                         Qt5::X11Extras Qt5::Widgets)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/session)
