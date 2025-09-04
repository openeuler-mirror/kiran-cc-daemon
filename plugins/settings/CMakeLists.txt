set(TARGET_NAME settings)

file(GLOB_RECURSE SETTINGS_H_FILES ./*.h)
file(GLOB_RECURSE SETTINGS_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  SETTINGS_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.Settings.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/settings-manager.h Kiran::SettingsManager)

add_library(${TARGET_NAME} SHARED ${SETTINGS_H_FILES} ${SETTINGS_CPP_FILES}
                                  ${SETTINGS_SRCS})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_BINARY_DIR}
          ${CMAKE_CURRENT_BINARY_DIR}
          ${GSETTINGS_QT_INCLUDE_DIRS}
          ${FONTCONFIG_INCLUDE_DIRS}
          ${GLIB2_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${GSETTINGS_QT_LIBRARIES} ${FONTCONFIG_LIBRARIES}
                         ${GLIB2_LIBRARIES} Qt5::Gui kbase kxcb)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/session)
