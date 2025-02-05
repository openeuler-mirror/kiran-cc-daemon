set(TARGET_NAME audio)

file(GLOB_RECURSE AUDIO_H_FILES ./*.h)
file(GLOB_RECURSE AUDIO_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  AUDIO_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.Audio.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/audio-manager.h Kiran::AudioManager)

qt5_add_dbus_adaptor(
  AUDIO_DEVICE_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.Audio.Device.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/audio-device.h Kiran::AudioDevice)

qt5_add_dbus_adaptor(
  AUDIO_STREAM_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.Audio.Stream.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/audio-stream.h Kiran::AudioStream)

add_library(
  ${TARGET_NAME} SHARED ${AUDIO_H_FILES} ${AUDIO_CPP_FILES} ${AUDIO_SRCS}
                        ${AUDIO_DEVICE_SRCS} ${AUDIO_STREAM_SRCS})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${LIBPULSE_INCLUDE_DIRS}
                         ${LIBPULSE_MAINLOOP_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${LIBPULSE_LIBRARIES} ${LIBPULSE_MAINLOOP_LIBRARIES}
                         kbase Qt5::Widgets)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/session)
