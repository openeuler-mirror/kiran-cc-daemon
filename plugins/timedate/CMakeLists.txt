cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME timedate)

file(GLOB_RECURSE TIMEDATE_H_FILES ./*.h)
file(GLOB_RECURSE TIMEDATE_CPP_FILES ./*.cpp)

pkg_search_module(SELINUX REQUIRED libselinux)


GEN_DBUS_STUB(TIMEDATE timedate ${CMAKE_SOURCE_DIR}/plugins/timedate/com.kylinsec.Kiran.SystemDaemon.TimeDate.xml)

add_subdirectory(po-timezones)

add_library(${TARGET_NAME} SHARED
    ${TIMEDATE_H_FILES}
    ${TIMEDATE_CPP_FILES}
    ${TIMEDATE_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_BINARY_DIR}/generated)


target_compile_definitions(${TARGET_NAME} PRIVATE
    -DKCC_LOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale")

target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GIOMM_LIBRARIES}
    lib-base
    lib-dbus)

if (SELINUX_MODULE_NAME)
    target_include_directories(${TARGET_NAME} PUBLIC 
        ${SELINUX_INCLUDE_DIRS})

    target_compile_options(${TARGET_NAME} PRIVATE
        -DHAVE_SELINUX)

    target_link_libraries(${TARGET_NAME} PRIVATE
        ${SELINUX_LIBRARIES})
endif()


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/system)


