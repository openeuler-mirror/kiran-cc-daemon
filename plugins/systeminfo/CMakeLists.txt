cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME systeminfo)

file(GLOB_RECURSE SYSTEMINFO_H_FILES ./*.h)
file(GLOB_RECURSE SYSTEMINFO_CPP_FILES ./*.cpp)

gen_dbus_stub(
  SYSTEMINFO
  systeminfo
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/systeminfo/com.kylinsec.Kiran.SystemDaemon.SystemInfo.xml
)

add_library(${TARGET_NAME} SHARED ${SYSTEMINFO_H_FILES} ${SYSTEMINFO_CPP_FILES}
                                  ${SYSTEMINFO_GENERATED_STUB})

set_common_shared_linker_flags()

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_BINARY_DIR}/generated
                                                 ${JSONCPP_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${KSD_LINKER_FLAGS} ${JSONCPP_LIBRARIES} lib-base
                         lib-dbus)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCC_PLUGIN_DIR}/system)
