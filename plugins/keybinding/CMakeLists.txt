cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME keybinding)

file(GLOB_RECURSE KEYBINDING_H_FILES ./*.h)
file(GLOB_RECURSE KEYBINDING_CPP_FILES ./*.cpp)

gen_dbus_stub(
  KEYBINDING
  keybinding
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/keybinding/com.kylinsec.Kiran.SessionDaemon.Keybinding.xml
)

add_library(${TARGET_NAME} SHARED ${KEYBINDING_H_FILES} ${KEYBINDING_CPP_FILES}
                                  ${KEYBINDING_GENERATED_STUB})

set_common_shared_linker_flags()

target_include_directories(
  ${TARGET_NAME} PUBLIC ${PROJECT_BINARY_DIR}/generated ${GTKMM3_INCLUDE_DIRS}
                        ${LIBXML2_INCLUDE_DIRS} ${LIBWNCK_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${KSD_LINKER_FLAGS}
          ${GTKMM3_LIBRARIES}
          ${LIBXML2_LIBRARIES}
          ${LIBWNCK_LIBRARIES}
          lib-base
          lib-display
          lib-osdwindow
          inputdevices-common)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCC_PLUGIN_DIR}/session)
