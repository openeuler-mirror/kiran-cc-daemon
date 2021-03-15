cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME greeter-settings)

file(GLOB_RECURSE GREETER_SETTINGS_H_FILES ./*.h)
file(GLOB_RECURSE GREETER_SETTINGS_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(GREETER_SETTINGS greeter-settings com.kylinsec. ${CMAKE_SOURCE_DIR}/plugins/greeter-settings/com.kylinsec.Kiran.SystemDaemon.GreeterSettings.xml)

add_library(${TARGET_NAME} SHARED
    ${GREETER_SETTINGS_H_FILES}
    ${GREETER_SETTINGS_CPP_FILES}
    ${GREETER_SETTINGS_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()

target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_BINARY_DIR}/generated
    ${CMAKE_SOURCE_DIR}/lib/base
    ${GDKMM3_INCLUDE_DIRS})


target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS})

target_link_libraries(${TARGET_NAME} PRIVATE
    lib-base)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/system)
