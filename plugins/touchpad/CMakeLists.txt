set(TARGET_NAME touchpad)

file(GLOB_RECURSE TOUCHPAD_H_FILES ./*.h)
file(GLOB_RECURSE TOUCHPAD_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  TOUCHPAD_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.TouchPad.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/touchpad-manager.h Kiran::TouchPadManager)

add_library(${TARGET_NAME} SHARED ${TOUCHPAD_H_FILES} ${TOUCHPAD_CPP_FILES}
                                  ${TOUCHPAD_SRCS})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}
          ${GSETTINGS_QT_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE kbase kinput)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/session)
