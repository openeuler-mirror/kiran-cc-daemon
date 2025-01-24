set(TARGET_NAME xsettings)

file(GLOB_RECURSE XSETTINGS_H_FILES ./*.h)
file(GLOB_RECURSE XSETTINGS_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  XSETTINGS_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.XSettings.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/xsettings-manager.h Kiran::XSettingsManager)

add_library(${TARGET_NAME} SHARED ${XSETTINGS_H_FILES} ${XSETTINGS_CPP_FILES}
                                  ${XSETTINGS_SRCS})

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
