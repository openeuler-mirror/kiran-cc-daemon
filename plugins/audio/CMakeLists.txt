cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME audio)

file(GLOB_RECURSE AUDIO_H_FILES ./*.h)
file(GLOB_RECURSE AUDIO_CPP_FILES ./*.cpp)

gen_dbus_stub(
  AUDIO audio com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/audio/com.kylinsec.Kiran.SessionDaemon.Audio.xml)
gen_dbus_stub(
  AUDIO_DEVICE
  audio_device
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/audio/com.kylinsec.Kiran.SessionDaemon.Audio.Device.xml
)
gen_dbus_stub(
  AUDIO_STREAM
  audio_stream
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/audio/com.kylinsec.Kiran.SessionDaemon.Audio.Stream.xml
)

add_library(
  ${TARGET_NAME} SHARED
  ${AUDIO_H_FILES} ${AUDIO_CPP_FILES} ${AUDIO_GENERATED_STUB}
  ${AUDIO_DEVICE_GENERATED_STUB} ${AUDIO_STREAM_GENERATED_STUB})

set_common_shared_linker_flags()

target_include_directories(
  ${TARGET_NAME} PUBLIC ${PROJECT_BINARY_DIR}/generated
                        ${LIBPULSE_INCLUDE_DIRS} ${JSONCPP_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${KSD_LINKER_FLAGS} ${LIBPULSE_LIBRARIES}
                         ${JSONCPP_LIBRARIES} lib-base)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCC_PLUGIN_DIR}/session)
