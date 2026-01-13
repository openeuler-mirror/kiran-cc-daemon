enable_testing()

set(TARGET_NAME kcd-test-audio)

file(GLOB_RECURSE TEST_H_FILES ./*.h)
file(GLOB_RECURSE TEST_CPP_FILES ./*.cpp)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/plugins/audio/com.kylinsec.Kiran.SessionDaemon.Audio.xml
  PROPERTIES CLASSNAME AudioProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  AUDIO_SRCS
  ${CMAKE_SOURCE_DIR}/plugins/audio/com.kylinsec.Kiran.SessionDaemon.Audio.xml
  audio_interface)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/plugins/audio/com.kylinsec.Kiran.SessionDaemon.Audio.Device.xml
  PROPERTIES CLASSNAME AudioDeviceProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  AUDIO_DEVICE_SRCS
  ${CMAKE_SOURCE_DIR}/plugins/audio/com.kylinsec.Kiran.SessionDaemon.Audio.Device.xml
  audio_device_interface)

add_executable(${TARGET_NAME} ${TEST_H_FILES} ${TEST_CPP_FILES} ${AUDIO_SRCS}
                              ${AUDIO_DEVICE_SRCS})

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include
                                                 ${CMAKE_BINARY_DIR})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Test kbase audio)

add_test(NAME test-audio COMMAND ${TARGET_NAME})
set_tests_properties(test-audio PROPERTIES LABELS "session-daemon")