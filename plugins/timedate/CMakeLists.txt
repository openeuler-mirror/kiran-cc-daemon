cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME timedate)

file(GLOB_RECURSE MOUSE_H_FILES ./*.h)
file(GLOB_RECURSE MOUSE_CPP_FILES ./*.cpp)

pkg_search_module(SELINUX REQUIRED libselinux)

SET (TIMEDATE_INTROSPECTION_XML ${CMAKE_SOURCE_DIR}/plugins/timedate/com.unikylin.Kiran.SystemDaemon.TimeDate.xml)

SET (TIMEDATE_GENERATED_STUB
    ${CMAKE_BINARY_DIR}/generated/timedate_dbus_stub.cpp
    ${CMAKE_BINARY_DIR}/generated/timedate_dbus_stub.h
    ${CMAKE_BINARY_DIR}/generated/timedate_dbus_common.cpp
    ${CMAKE_BINARY_DIR}/generated/timedate_dbus_common.h
)

ADD_CUSTOM_COMMAND (OUTPUT ${TIMEDATE_GENERATED_STUB}
                    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/generated/
                    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_BINARY_DIR}/lib/python3.6/site-packages/ 
                            ${GDBUS_CODEGEN} --generate-cpp-code=${CMAKE_BINARY_DIR}/generated/timedate_dbus
                                        --interface-prefix=com.unikylin.
                                        ${TIMEDATE_INTROSPECTION_XML}
                    DEPENDS ${TIMEDATE_INTROSPECTION_XML}
                    COMMENT "Generate the stub for the timedate")




add_library(${TARGET_NAME} SHARED
    ${MOUSE_H_FILES}
    ${MOUSE_CPP_FILES}
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
        DESTINATION  ${KSD_PLUGIN_DIR})

install(FILES "timedate.ksd-plugin.in" 
        DESTINATION  ${KSD_PLUGIN_DIR}
        RENAME "timedate.ksd-plugin")

install(FILES "com.unikylin.Kiran.SystemDaemon.TimeDate.conf"
        DESTINATION ${SYSCONFDIR}/dbus-1/system.d)
