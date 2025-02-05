set(TARGET_NAME mouse)

file(GLOB_RECURSE MOUSE_H_FILES ./*.h)
file(GLOB_RECURSE MOUSE_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  MOUSE_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.Mouse.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/mouse-manager.h Kiran::MouseManager)

add_library(${TARGET_NAME} SHARED ${MOUSE_H_FILES} ${MOUSE_CPP_FILES}
                                  ${MOUSE_SRCS})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}
          ${GSETTINGS_QT_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE kbase kinput)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/session)
