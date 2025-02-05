set(TARGET_NAME systeminfo)

file(GLOB_RECURSE SYSTEMINFO_H_FILES ./*.h)
file(GLOB_RECURSE SYSTEMINFO_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  SYSTEMINFO_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SystemDaemon.SystemInfo.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/systeminfo-manager.h Kiran::SystemInfoManager)

add_library(${TARGET_NAME} SHARED ${SYSTEMINFO_H_FILES} ${SYSTEMINFO_CPP_FILES}
                                  ${SYSTEMINFO_SRCS})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
         ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}
         ${LIBGTOP_INCLUDE_DIRS} ${GUDEV_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${KSD_LINKER_FLAGS} ${JSONCPP_LIBRARIES}
                         ${LIBGTOP_LIBRARIES} ${GUDEV_LIBRARIES} kbase)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/system)
