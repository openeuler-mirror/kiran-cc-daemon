set(TARGET_NAME kiran-power-backlight-helper)

file(GLOB_RECURSE POWER_BACKLIGHT_HELPER_H_FILES ./*.h)
file(GLOB_RECURSE POWER_BACKLIGHT_HELPER_CPP_FILES ./*.cpp)

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/kiran-power-backlight-helper.zh_CN.ts")

execute_process(
  COMMAND lupdate-qt5 -no-obsolete ${POWER_BACKLIGHT_HELPER_H_FILES}
          ${POWER_BACKLIGHT_HELPER_CPP_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(BACKLIGHT_QM_FILES ${TS_FILES})

add_executable(
  ${TARGET_NAME}
  ${POWER_BACKLIGHT_HELPER_H_FILES}
  ${POWER_BACKLIGHT_HELPER_CPP_FILES}
  ${BACKLIGHT_QM_FILES}
  ${PROJECT_SOURCE_DIR}/plugins/power/wrapper/power-upower.cpp
  ${PROJECT_SOURCE_DIR}/plugins/power/wrapper/power-upower-device.cpp)

target_link_libraries(${TARGET_NAME} PRIVATE kbase)

install(FILES ${BACKLIGHT_QM_FILES} DESTINATION ${KCD_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}/)
