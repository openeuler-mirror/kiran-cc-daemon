set(TARGET_NAME timedate)

file(GLOB_RECURSE TIMEDATE_H_FILES ./*.h)
file(GLOB_RECURSE TIMEDATE_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  TIMEDATE_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SystemDaemon.TimeDate.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/timedate-manager.h Kiran::TimedateManager)

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts"
)
execute_process(COMMAND lupdate-qt5 -no-obsolete ${TIMEDATE_H_FILES}
                        ${TIMEDATE_CPP_FILES} -ts ${TS_FILES})
qt5_add_translation(QM_FILES ${TS_FILES})

set(TIME_ZONES_TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/${PROJECT_NAME}-timezones.zh_CN.ts")
execute_process(
  COMMAND lupdate-qt5 -no-obsolete ${CMAKE_CURRENT_SOURCE_DIR}/timezones.txt
          -ts ${TIME_ZONES_TS_FILES})
qt5_add_translation(TIME_ZONES_QM_FILES ${TIME_ZONES_TS_FILES})

add_library(
  ${TARGET_NAME} SHARED ${TIMEDATE_H_FILES} ${TIMEDATE_CPP_FILES}
                        ${TIMEDATE_SRCS} ${QM_FILES} ${TIME_ZONES_QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}
          ${GLIB2_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${GLIB2_LIBRARIES} kbase)

if(SELINUX_MODULE_NAME)
  target_include_directories(${TARGET_NAME} PUBLIC ${SELINUX_INCLUDE_DIRS})

  target_compile_options(${TARGET_NAME} PRIVATE -DHAVE_SELINUX)

  target_link_libraries(${TARGET_NAME} PRIVATE ${SELINUX_LIBRARIES})
endif()

install(
  FILES ${PROJECT_SOURCE_DIR}/plugins/timedate/timedate.conf
  DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME}/system/timedate/)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/system)
install(FILES ${QM_FILES} ${TIME_ZONES_QM_FILES}
        DESTINATION ${KCD_INSTALL_TRANSLATIONDIR})
