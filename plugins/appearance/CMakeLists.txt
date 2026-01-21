set(TARGET_NAME appearance)

file(GLOB_RECURSE APPEARANCE_H_FILES ./*.h)
file(GLOB_RECURSE APPEARANCE_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  APPEARANCE_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.Appearance.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/appearance-manager.h Kiran::AppearanceManager)

add_library(${TARGET_NAME} SHARED ${APPEARANCE_H_FILES} ${APPEARANCE_CPP_FILES}
                                  ${APPEARANCE_SRCS})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
                         ${GSETTINGS_QT_INCLUDE_DIRS} ${XCB_IMAGE_INCLUDE_DIRS})

if(X11_Xcursor_FOUND)
  message(STATUS "Xcursor found, enabling Xcursor support")
  add_definitions(-DHAVE_XCURSOR)
  target_link_libraries(${TARGET_NAME} PRIVATE X11::Xcursor)
endif()

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${GSETTINGS_QT_LIBRARIES} ${XCB_IMAGE_LIBRARIES} kbase
                         kxcb Qt5::Widgets Qt5::X11Extras)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/session)
