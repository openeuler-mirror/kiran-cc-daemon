cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME timedate)

file(GLOB_RECURSE TIMEDATE_H_FILES ./*.h)
file(GLOB_RECURSE TIMEDATE_CPP_FILES ./*.cpp)

pkg_search_module(SELINUX REQUIRED libselinux)

gen_dbus_stub(
  TIMEDATE
  timedate
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/timedate/com.kylinsec.Kiran.SystemDaemon.TimeDate.xml
)

add_subdirectory(po-timezones)

add_library(${TARGET_NAME} SHARED ${TIMEDATE_H_FILES} ${TIMEDATE_CPP_FILES}
                                  ${TIMEDATE_GENERATED_STUB})

set_common_shared_linker_flags()

target_include_directories(${TARGET_NAME}
                           PUBLIC ${PROJECT_BINARY_DIR}/generated)

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${KSD_LINKER_FLAGS} ${GIOMM_LIBRARIES} lib-base
                         lib-dbus)

if(SELINUX_MODULE_NAME)
  target_include_directories(${TARGET_NAME} PUBLIC ${SELINUX_INCLUDE_DIRS})

  target_compile_options(${TARGET_NAME} PRIVATE -DHAVE_SELINUX)

  target_link_libraries(${TARGET_NAME} PRIVATE ${SELINUX_LIBRARIES})
endif()

install(FILES ${PROJECT_SOURCE_DIR}/plugins/timedate/timedate.conf
        DESTINATION ${SYSCONFDIR}/${PROJECT_NAME}/system/timedate/)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCC_PLUGIN_DIR}/system)
