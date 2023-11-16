cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME keyboard)

file(GLOB_RECURSE KEYBOARD_H_FILES ./*.h)
file(GLOB_RECURSE KEYBOARD_CPP_FILES ./*.cpp)

gen_dbus_stub(
  KEYBOARD
  keyboard
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/inputdevices/keyboard/com.kylinsec.Kiran.SessionDaemon.Keyboard.xml
)

add_library(${TARGET_NAME} SHARED ${KEYBOARD_H_FILES} ${KEYBOARD_CPP_FILES}
                                  ${KEYBOARD_GENERATED_STUB})

set_common_shared_linker_flags()

pkg_get_variable(XKB_DATA_ROOT_DIR xorg-macros datarootdir)
if(NOT XKB_DATA_ROOT_DIR)
  set(XKB_DATA_ROOT_DIR "/usr/share")
endif()
set(XKB_BASE "${XKB_DATA_ROOT_DIR}/X11/xkb")

target_include_directories(
  ${TARGET_NAME} PUBLIC ${PROJECT_BINARY_DIR}/generated ${GDKMM3_INCLUDE_DIRS}
                        ${LIBXML2_INCLUDE_DIRS} ${GTKMM3_INCLUDE_DIRS})

target_compile_definitions(${TARGET_NAME} PRIVATE -DKCC_XKB_BASE="${XKB_BASE}")

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${KSD_LINKER_FLAGS}
          ${GDKMM3_LIBRARIES}
          ${LIBXML2_LIBRARIES}
          ${GTKMM3_LIBRARIES}
          inputdevices-common
          lib-base
          lib-iso
          lib-osdwindow)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCC_PLUGIN_DIR}/session)
