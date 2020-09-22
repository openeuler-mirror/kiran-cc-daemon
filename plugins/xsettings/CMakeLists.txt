cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME xsettings)

file(GLOB_RECURSE XSETTINGS_H_FILES ./*.h)
file(GLOB_RECURSE XSETTINGS_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(XSETTINGS xsettings ${CMAKE_SOURCE_DIR}/plugins/xsettings/com.unikylin.Kiran.SessionDaemon.XSettings.xml)


add_library(${TARGET_NAME} SHARED
    ${XSETTINGS_H_FILES}
    ${XSETTINGS_CPP_FILES}
    ${XSETTINGS_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_BINARY_DIR}/generated)


target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS})

target_link_libraries(${TARGET_NAME} PRIVATE
    lib-base)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/session)
