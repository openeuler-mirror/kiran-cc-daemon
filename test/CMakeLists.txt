enable_testing()

set(TARGET_NAME kiran-cc-test)

file(GLOB_RECURSE TEST_H_FILES ./*.h)
file(GLOB_RECURSE TEST_CPP_FILES ./*.cpp)

gen_dbus_stub(
  ACCOUNTS
  accounts
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/accounts/com.kylinsec.Kiran.SystemDaemon.Accounts.xml
)
gen_dbus_stub(
  USER
  user
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/accounts/com.kylinsec.Kiran.SystemDaemon.Accounts.User.xml
)

gen_dbus_stub(
  GROUPS
  groups
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/groups/com.kylinsec.Kiran.SystemDaemon.Groups.xml
)

gen_dbus_stub(
  GROUP
  group
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/groups/com.kylinsec.Kiran.SystemDaemon.Groups.Group.xml
)

gen_dbus_stub(
  APPEARANCE
  appearance
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/appearance/com.kylinsec.Kiran.SessionDaemon.Appearance.xml
)

gen_dbus_stub(
  AUDIO
  audio
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/audio/com.kylinsec.Kiran.SessionDaemon.Audio.xml
)

gen_dbus_stub(
  AUDIO_DEVICE
  audio_device
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/audio/com.kylinsec.Kiran.SessionDaemon.Audio.Device.xml
)

gen_dbus_stub(
  SYSTEMINFO
  systeminfo
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/systeminfo/com.kylinsec.Kiran.SystemDaemon.SystemInfo.xml
)

gen_dbus_stub(
  TIMEDATE
  timedate
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/timedate/com.kylinsec.Kiran.SystemDaemon.TimeDate.xml
)

add_executable(${TARGET_NAME} ${TEST_H_FILES} ${TEST_CPP_FILES}
                              ${ACCOUNTS_GENERATED_STUB} ${USER_GENERATED_STUB}
                              ${GROUPS_GENERATED_STUB} ${GROUP_GENERATED_STUB}
                              ${APPEARANCE_GENERATED_STUB}
                              ${AUDIO_GENERATED_STUB} ${AUDIO_DEVICE_GENERATED_STUB}
                              ${SYSTEMINFO_GENERATED_STUB}
                              ${TIMEDATE_GENERATED_STUB})

# gtest >= 1.13 需要 c++14（1.13.0 起最低要求 C++14，1.12 及以下用 C++11 即可）
if(GTEST_VERSION AND GTEST_VERSION VERSION_GREATER_EQUAL "1.13")
  set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)
endif()

target_include_directories(
  ${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include
                        ${PROJECT_BINARY_DIR}/generated ${GTEST_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${GTEST_LIBRARIES} lib-base)

add_test(NAME test COMMAND ${TARGET_NAME})
