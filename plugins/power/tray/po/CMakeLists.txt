
cmake_minimum_required(VERSION 3.0)

find_program(MSGFMT NAMES "msgfmt" REQUIRED)
find_program(INTLTOOL-UPDATE NAMES "intltool-update" REQUIRED)

#execute_process(COMMAND ${INTLTOOL-UPDATE} zh_CN
#                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/po)

add_custom_target(POWER_TRAY_ZH_MO ALL ${MSGFMT} zh_CN.po -D ${PROJECT_SOURCE_DIR}/plugins/power/tray/po/ -o ${PROJECT_BINARY_DIR}/plugins/power/tray/po/zh_CN.mo)

install(FILES ${PROJECT_BINARY_DIR}/plugins/power/tray/po/zh_CN.mo
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/locale/zh_CN/LC_MESSAGES/"
        RENAME kiran-power-status-icon.mo)