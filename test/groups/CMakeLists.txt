enable_testing()

set(TARGET_NAME kcd-test-groups)

file(GLOB_RECURSE TEST_H_FILES ./*.h)
file(GLOB_RECURSE TEST_CPP_FILES ./*.cpp)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/plugins/accounts/com.kylinsec.Kiran.SystemDaemon.Accounts.xml
  PROPERTIES CLASSNAME AccountsProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  ACCOUNTS_SRCS
  ${CMAKE_SOURCE_DIR}/plugins/accounts/com.kylinsec.Kiran.SystemDaemon.Accounts.xml
  accounts_interface)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/plugins/accounts/com.kylinsec.Kiran.SystemDaemon.Accounts.User.xml
  PROPERTIES CLASSNAME UserProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  USERS_SRCS
  ${CMAKE_SOURCE_DIR}/plugins/accounts/com.kylinsec.Kiran.SystemDaemon.Accounts.User.xml
  user_interface)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/plugins/groups/com.kylinsec.Kiran.SystemDaemon.Groups.xml
  PROPERTIES CLASSNAME GroupsProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  GROUPS_SRCS
  ${CMAKE_SOURCE_DIR}/plugins/groups/com.kylinsec.Kiran.SystemDaemon.Groups.xml
  groups_interface)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/plugins/groups/com.kylinsec.Kiran.SystemDaemon.Groups.Group.xml
  PROPERTIES CLASSNAME GroupProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  GROUP_SRCS
  ${CMAKE_SOURCE_DIR}/plugins/groups/com.kylinsec.Kiran.SystemDaemon.Groups.Group.xml
  group_interface)

add_executable(
  ${TARGET_NAME}
  ${TEST_H_FILES}
  ${TEST_CPP_FILES}
  ${ACCOUNTS_SRCS}
  ${USERS_SRCS}
  ${GROUPS_SRCS}
  ${GROUP_SRCS}
  ${USER_SRCS})

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include
                                                 ${CMAKE_BINARY_DIR})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Test kbase groups accounts)

add_test(NAME test-groups COMMAND ${TARGET_NAME})
set_tests_properties(test-groups PROPERTIES LABELS "system-daemon")
