cmake_minimum_required(VERSION 3.2)

enable_testing()

set(TARGET_NAME kiran-cc-test)

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

add_executable(${TARGET_NAME} ${CMAKE_SOURCE_DIR}/test/test-system-daemon.cpp
                              ${ACCOUNTS_SRCS} ${USERS_SRCS})

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include
                                                 ${CMAKE_BINARY_DIR})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Test kbase)

add_test(NAME test COMMAND ${TARGET_NAME})
