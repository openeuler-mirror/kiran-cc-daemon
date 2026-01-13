enable_testing()

set(TARGET_NAME kcd-test-timedate)

file(GLOB_RECURSE TEST_H_FILES ./*.h)
file(GLOB_RECURSE TEST_CPP_FILES ./*.cpp)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/plugins/timedate/com.kylinsec.Kiran.SystemDaemon.TimeDate.xml
  PROPERTIES CLASSNAME TimedateProxy NO_NAMESPACE true)

set_property(
  SOURCE
    ${CMAKE_SOURCE_DIR}/plugins/timedate/com.kylinsec.Kiran.SystemDaemon.TimeDate.xml
  PROPERTY INCLUDE ${PROJECT_SOURCE_DIR}/plugins/timedate/dbus-types.h)

qt5_add_dbus_interface(
  TIMEDATE_SRCS
  ${CMAKE_SOURCE_DIR}/plugins/timedate/com.kylinsec.Kiran.SystemDaemon.TimeDate.xml
  timedate_interface)

add_executable(${TARGET_NAME} ${TEST_H_FILES} ${TEST_CPP_FILES}
                              ${TIMEDATE_SRCS})

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include
                                                 ${CMAKE_BINARY_DIR})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Test kbase timedate)

add_test(NAME test-timedate COMMAND ${TARGET_NAME})
set_tests_properties(test-timedate PROPERTIES LABELS "system-daemon")
