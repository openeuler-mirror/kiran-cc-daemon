set(TARGET_NAME groups)

file(GLOB_RECURSE GROUPS_H_FILES ./*.h)
file(GLOB_RECURSE GROUPS_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  GROUPS_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SystemDaemon.Groups.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/groups-manager.h Kiran::GroupsManager)

qt5_add_dbus_adaptor(
  GROUP_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SystemDaemon.Groups.Group.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/group.h Kiran::Group)


# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts"
)

execute_process(COMMAND lupdate-qt5 -no-obsolete ${GROUPS_H_FILES}
                        ${GROUPS_CPP_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

add_library(${TARGET_NAME} SHARED ${GROUPS_H_FILES} ${GROUPS_CPP_FILES}
                                  ${GROUPS_SRCS} ${GROUP_SRCS} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}
          ${GLIB2_INCLUDE_DIRS})

target_link_libraries( ${TARGET_NAME} PRIVATE ${GLIB2_LIBRARIES} kbase Qt5::DBus)

install(FILES ${QM_FILES} DESTINATION ${KCD_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/system)
