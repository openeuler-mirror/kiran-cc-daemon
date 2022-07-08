cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME xsettings)

file(GLOB_RECURSE XSETTINGS_H_FILES ./*.h)
file(GLOB_RECURSE XSETTINGS_CPP_FILES ./*.cpp)

gen_dbus_stub(
  XSETTINGS
  xsettings
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/xsettings/com.kylinsec.Kiran.SessionDaemon.XSettings.xml
)

add_library(${TARGET_NAME} SHARED ${XSETTINGS_H_FILES} ${XSETTINGS_CPP_FILES}
                                  ${XSETTINGS_GENERATED_STUB})

set_common_shared_linker_flags()

target_include_directories(
  ${TARGET_NAME} PUBLIC ${PROJECT_BINARY_DIR}/generated ${GDKMM3_INCLUDE_DIRS}
                        ${FONTCONFIG_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${KSD_LINKER_FLAGS} ${GDKMM3_LIBRARIES}
                         ${FONTCONFIG_LIBRARIES} lib-base lib-display)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCC_PLUGIN_DIR}/session)
