set(TARGET_NAME upgrade)

file(GLOB UPGRADE_H_FILES ./*.h dnf/*.h)
file(GLOB UPGRADE_CPP_FILES ./*.cpp dnf/*.cpp)

qt5_add_dbus_adaptor(
  UPGRADE_SRCS
  ${PROJECT_SOURCE_DIR}/plugins/upgrade/com.kylinsec.Kiran.SystemDaemon.Upgrade.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/manager.h Kiran::Manager)


# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts"
)

execute_process(COMMAND lupdate-qt5 -no-obsolete ${UPGRADE_H_FILES}
                        ${UPGRADE_CPP_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

add_library(${TARGET_NAME} SHARED ${UPGRADE_H_FILES} 
                                  ${UPGRADE_CPP_FILES}
                                  ${UPGRADE_SRCS}
                                  ${QM_FILES})

target_link_libraries(${TARGET_NAME} PRIVATE 
    Qt5::Concurrent
    Qt5::Widgets
    kbase
    ${GLIB2_LIBRARIES}
    ${LIBDNF_LIBRARIES}
    ${GOBJECT2_LIBRARIES}
)
target_include_directories(${TARGET_NAME} PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${GLIB2_INCLUDE_DIRS}
    ${LIBDNF_INCLUDE_DIRS}
    ${GOBJECT2_INCLUDE_DIRS}
)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/system)
install(
  FILES ${PROJECT_SOURCE_DIR}/plugins/upgrade/upgrade.conf
  DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME}/system/upgrade/)

install(FILES ${QM_FILES} DESTINATION ${KCD_INSTALL_TRANSLATIONDIR})

add_subdirectory(notify)