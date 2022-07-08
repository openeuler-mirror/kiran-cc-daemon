cmake_minimum_required(VERSION 3.0)

find_program(MSGFMT NAMES "msgfmt" REQUIRED)

add_custom_target(
  TIMEZONES_MO ALL
  ${MSGFMT} zh_CN.po -D ${PROJECT_SOURCE_DIR}/plugins/timedate/po-timezones -o
  ${PROJECT_BINARY_DIR}/plugins/timedate/po-timezones/zh_CN.mo)

install(
  FILES ${PROJECT_BINARY_DIR}/plugins/timedate/po-timezones/zh_CN.mo
  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/locale/zh_CN/LC_MESSAGES/"
  RENAME ${PROJECT_NAME}-timezones.mo)
