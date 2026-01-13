enable_testing()

set(TARGET_NAME kcd-test-appearance)

file(GLOB_RECURSE TEST_H_FILES ./*.h)
file(GLOB_RECURSE TEST_CPP_FILES ./*.cpp)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/plugins/appearance/com.kylinsec.Kiran.SessionDaemon.Appearance.xml
  PROPERTIES CLASSNAME AppearanceProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  APPEARANCE_SRCS
  ${CMAKE_SOURCE_DIR}/plugins/appearance/com.kylinsec.Kiran.SessionDaemon.Appearance.xml
  appearance_interface)

add_executable(${TARGET_NAME} ${TEST_H_FILES} ${TEST_CPP_FILES}
                              ${APPEARANCE_SRCS})

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include
                                                 ${CMAKE_BINARY_DIR})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Test kbase appearance)

add_test(NAME test-appearance COMMAND ${TARGET_NAME})
set_tests_properties(test-appearance PROPERTIES LABELS "session-daemon")
