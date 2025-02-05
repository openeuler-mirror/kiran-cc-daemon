set(TARGET_NAME greeter)

file(GLOB_RECURSE GREETER_H_FILES ./*.h)
file(GLOB_RECURSE GREETER_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  GREETER_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SystemDaemon.Greeter.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/greeter-manager.h Kiran::GreeterManager)

add_library(${TARGET_NAME} SHARED ${GREETER_H_FILES} ${GREETER_CPP_FILES}
                                  ${GREETER_SRCS})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
                         ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${TARGET_NAME} PRIVATE KF5::ConfigCore kbase)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/system)

install(FILES ${PROJECT_SOURCE_DIR}/plugins/greeter/kiran-greeter.conf
        DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/lightdm/)

install(FILES ${PROJECT_SOURCE_DIR}/plugins/greeter/99-kiran-greeter-login.conf
        DESTINATION /usr/share/lightdm/lightdm.conf.d/)
