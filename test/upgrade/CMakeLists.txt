enable_testing()

set(TARGET_NAME kcd-test-upgrade)

file(GLOB_RECURSE TEST_H_FILES ./*.h)
file(GLOB_RECURSE TEST_CPP_FILES ./*.cpp)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/plugins/upgrade/com.kylinsec.Kiran.SystemDaemon.Upgrade.xml
  PROPERTIES CLASSNAME UpgradeProxy NO_NAMESPACE true INCLUDE upgrade-i.h)

qt5_add_dbus_interface(
  UPGRADE_SRCS
  ${CMAKE_SOURCE_DIR}/plugins/upgrade/com.kylinsec.Kiran.SystemDaemon.Upgrade.xml
  upgrade_interface)

add_executable(${TARGET_NAME} ${TEST_H_FILES} ${TEST_CPP_FILES}
                              ${UPGRADE_SRCS})

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include
                                                 ${CMAKE_BINARY_DIR})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Test Qt5::DBus kbase upgrade)

add_test(NAME test-upgrade COMMAND ${TARGET_NAME})
set_tests_properties(test-upgrade PROPERTIES LABELS "system-daemon")

