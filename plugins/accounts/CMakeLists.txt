set(TARGET_NAME accounts)

file(GLOB_RECURSE ACCOUNTS_H_FILES ./*.h)
file(GLOB_RECURSE ACCOUNTS_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  ACCOUNTS_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SystemDaemon.Accounts.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/accounts-manager.h Kiran::AccountsManager)

qt5_add_dbus_adaptor(
  USER_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SystemDaemon.Accounts.User.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/user.h Kiran::User)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-accounts.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config-accounts.h)

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts"
)

execute_process(COMMAND lupdate-qt5 -no-obsolete ${ACCOUNTS_H_FILES}
                        ${ACCOUNTS_CPP_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

add_library(${TARGET_NAME} SHARED ${ACCOUNTS_H_FILES} ${ACCOUNTS_CPP_FILES}
                                  ${ACCOUNTS_SRCS} ${USER_SRCS} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}
          ${GLIB2_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE KF5::ConfigCore kbase ${LIBXCRYPT_LIBRARIES}
                         ${GLIB2_LIBRARIES})

install(FILES ${QM_FILES} DESTINATION ${KCD_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/system)
