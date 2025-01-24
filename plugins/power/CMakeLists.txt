set(TARGET_NAME power)

file(GLOB_RECURSE POWER_H_FILES ./*.h)
file(GLOB_RECURSE POWER_CPP_FILES ./*.cpp)

# Filter tools directory list(FILTER POWER_H_FILES EXCLUDE REGEX .*tools/.*)
# list(FILTER POWER_CPP_FILES EXCLUDE REGEX .*tools/.*)
list(FILTER POWER_CPP_FILES EXCLUDE REGEX "(.*tools/.*|.*/tray/.*)")

qt5_add_dbus_adaptor(
  POWER_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.Power.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/power-manager.h Kiran::PowerManager)

add_library(${TARGET_NAME} SHARED ${POWER_H_FILES} ${POWER_CPP_FILES}
                                  ${POWER_SRCS})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_BINARY_DIR}
          ${CMAKE_CURRENT_BINARY_DIR}
          ${GLIB2_INCLUDE_DIRS}
          ${LIBNOTIFY_INCLUDE_DIRS}
          ${XCB_INCLUDE_DIRS}
          ${GSETTINGS_QT_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${XCB_LIBRARIES}
          ${LIBNOTIFY_LIBRARIES}
          ${GSETTINGS_QT_LIBRARIES}
          kbase
          Qt5::X11Extras
          KF5::GlobalAccel
          KF5::XmlGui
          KF5::Screen)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/session)

# add_subdirectory(tools) add_subdirectory(tray)
