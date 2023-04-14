cmake_minimum_required(VERSION 3.0)

find_program(MSGFMT NAMES "msgfmt" REQUIRED)
find_program(INTLTOOL-UPDATE NAMES "intltool-update" REQUIRED)

# execute_process(COMMAND ${INTLTOOL-UPDATE} zh_CN WORKING_DIRECTORY
# ${PROJECT_SOURCE_DIR}/po)

file(GLOB TS_FILES *.po)
foreach(TS_FILE ${TS_FILES})
    string(REGEX REPLACE .*/ "" tmp ${TS_FILE})
    string(REGEX REPLACE \\..* "" TS_FILE_NAME ${tmp})
    add_custom_target(${TARGET_NAME}_${TS_FILE_NAME}_MO ALL ${MSGFMT}
        ${TS_FILE} -D ${PROJECT_SOURCE_DIR}/plugins/power/tray/po/ -o
        ${PROJECT_BINARY_DIR}/plugins/power/tray/po/${TARGET_NAME}_${TS_FILE_NAME}.mo)
    install(
        FILES ${PROJECT_BINARY_DIR}/plugins/power/tray/po/${TARGET_NAME}_${TS_FILE_NAME}.mo
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/locale/${TS_FILE_NAME}/LC_MESSAGES/"
        RENAME kiran-power-status-icon.mo)
endforeach()
