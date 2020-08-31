cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME keyboard)

file(GLOB_RECURSE KEYBOARD_H_FILES ./*.h)
file(GLOB_RECURSE KEYBOARD_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(KEYBOARD keyboard ${CMAKE_SOURCE_DIR}/plugins/inputdevices/keyboard/com.unikylin.Kiran.SessionDaemon.Keyboard.xml)


add_library(${TARGET_NAME} SHARED
    ${KEYBOARD_H_FILES}
    ${KEYBOARD_CPP_FILES}
    ${KEYBOARD_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()

pkg_get_variable(XKB_DATA_ROOT_DIR xorg-macros datarootdir)
if (NOT XKB_DATA_ROOT_DIR)
    set(XKB_DATA_ROOT_DIR "/usr/share")
endif()
set(XKB_BASE "${XKB_DATA_ROOT_DIR}/X11/xkb")

target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/generated
    ${GDKMM3_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS})


target_compile_definitions(${TARGET_NAME} PRIVATE
    -DKCC_XKB_BASE="${XKB_BASE}")

target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS}
    -L${LIBDIR})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GDKMM3_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    inputdevices-common
    fmt)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/session)

install(FILES "keyboard.ksd-plugin.in" 
        DESTINATION  ${KCC_PLUGIN_DIR}/session
        RENAME "keyboard.ksd-plugin")