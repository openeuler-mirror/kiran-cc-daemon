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
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/generated
    ${GIOMM_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/third_party/fmt-5.2.0/include)


target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS}
    -L${LIBDIR})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GIOMM_LIBRARIES}
    kiran-lib
    fmt)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/session)

install(FILES "xsettings.ksd-plugin.in" 
        DESTINATION  ${KCC_PLUGIN_DIR}/session
        RENAME "xsettings.ksd-plugin")


