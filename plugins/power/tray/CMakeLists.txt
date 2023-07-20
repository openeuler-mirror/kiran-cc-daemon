cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME kiran-power-status-icon)

file(GLOB_RECURSE POWER_TRAY_H_FILES ./*.h)
file(GLOB_RECURSE POWER_TRAY_CPP_FILES ./*.cpp)

add_executable(
  ${TARGET_NAME}
  ${POWER_TRAY_H_FILES} ${POWER_TRAY_CPP_FILES}
  ${PROJECT_SOURCE_DIR}/plugins/power/wrapper/power-upower.cpp
  ${PROJECT_SOURCE_DIR}/plugins/power/wrapper/power-upower-device.cpp
  ${PROJECT_SOURCE_DIR}/plugins/power/power-utils.cpp)

target_include_directories(${TARGET_NAME} PUBLIC ${GTKMM3_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${GTKMM3_LIBRARIES} lib-base)

install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}/)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kiran-power-status-icon.desktop.in
               ${CMAKE_CURRENT_BINARY_DIR}/kiran-power-status-icon.desktop)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kiran-power-status-icon.desktop
        DESTINATION ${SYSCONFDIR}/xdg/autostart/)

add_subdirectory(icons)
add_subdirectory(po)
