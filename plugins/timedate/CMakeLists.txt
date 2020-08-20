cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME timedate)

file(GLOB_RECURSE TIMEDATE_H_FILES ./*.h)
file(GLOB_RECURSE TIMEDATE_CPP_FILES ./*.cpp)

pkg_search_module(SELINUX REQUIRED libselinux)


GEN_DBUS_STUB(TIMEDATE timedate ${CMAKE_SOURCE_DIR}/plugins/timedate/com.unikylin.Kiran.SystemDaemon.TimeDate.xml)


add_library(${TARGET_NAME} SHARED
    ${TIMEDATE_H_FILES}
    ${TIMEDATE_CPP_FILES}
    ${TIMEDATE_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/generated
    ${GLIBMM_INCLUDE_DIRS}
    ${GIOMM_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/third_party/fmt-5.2.0/include)


target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS}
    -L${LIBDIR})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GLIBMM_LIBRARIES}
    ${GIOMM_LIBRARIES}
    kiran-lib
    fmt)

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

install(FILES "timedate.ksd-plugin.in" 
        DESTINATION  ${KCC_PLUGIN_DIR}/system
        RENAME "timedate.ksd-plugin")


