enable_testing()

set(TARGET_NAME kcd-test-systeminfo)

file(GLOB_RECURSE TEST_H_FILES ./*.h)
file(GLOB_RECURSE TEST_CPP_FILES ./*.cpp)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/plugins/systeminfo/com.kylinsec.Kiran.SystemDaemon.SystemInfo.xml
  PROPERTIES CLASSNAME SysteminfoProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  SYSTEMINFO_SRCS
  ${CMAKE_SOURCE_DIR}/plugins/systeminfo/com.kylinsec.Kiran.SystemDaemon.SystemInfo.xml
  systeminfo_interface)

add_executable(${TARGET_NAME} ${TEST_H_FILES} ${TEST_CPP_FILES}
                              ${SYSTEMINFO_SRCS})

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include
                                                 ${CMAKE_BINARY_DIR})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Test kbase systeminfo)

add_test(NAME test-systeminfo COMMAND ${TARGET_NAME})
set_tests_properties(test-systeminfo PROPERTIES LABELS "system-daemon")
