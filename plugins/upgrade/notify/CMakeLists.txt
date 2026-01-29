
set(TARGET_NAME kcd-upgrade-notify)

find_package(KiranDBusGenerate REQUIRED)

file(GLOB_RECURSE NOTIFY_H_FILES ./*.h)
file(GLOB_RECURSE NOTIFY_CPP_FILES ./*.cpp)

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/plugins/upgrade/notify/translations/${TARGET_NAME}.zh_CN.ts"
)

execute_process(COMMAND lupdate-qt5 -no-obsolete ${NOTIFY_H_FILES}
                        ${NOTIFY_CPP_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

set(KSD_UPGRADE_XML ${PROJECT_SOURCE_DIR}/plugins/upgrade/com.kylinsec.Kiran.SystemDaemon.Upgrade.xml)
set_source_files_properties(${KSD_UPGRADE_XML}
        PROPERTIES
        CLASSNAME KSDUpgradeProxy
        NO_NAMESPACE true
        INCLUDE upgrade-i.h)
kiran_qt5_add_dbus_interface(KSD_UPGRADE_SRC ${KSD_UPGRADE_XML} ksd_upgrade_proxy)

add_executable(${TARGET_NAME} 
            ${NOTIFY_H_FILES} ${NOTIFY_CPP_FILES} ${KSD_UPGRADE_SRC} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
                         ${PROJECT_SOURCE_DIR}/plugins/upgrade
                         ${CMAKE_CURRENT_BINARY_DIR} ${KLOG_QT5_INCLUDE_DIRS}
                         ${LIBNOTIFY_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Core Qt5::Network kbase 
                ${KLOG_QT5_LIBRARIES} ${LIBNOTIFY_LIBRARIES})

install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
install(FILES ${QM_FILES} DESTINATION ${KCD_INSTALL_TRANSLATIONDIR})