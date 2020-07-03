cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME accounts)

file(GLOB_RECURSE MOUSE_H_FILES ./*.h)
file(GLOB_RECURSE MOUSE_CPP_FILES ./*.cpp)

pkg_search_module(ACCOUNTS_SERVICE REQUIRED accountsservice)


SET (ACCOUNTS_INTROSPECTION_XML ${CMAKE_SOURCE_DIR}/plugins/accounts/com.unikylin.Kiran.System.Accounts.xml)

SET (ACCOUNTS_GENERATED_STUB
    ${CMAKE_BINARY_DIR}/generated/accounts_dbus_stub.cpp
    ${CMAKE_BINARY_DIR}/generated/accounts_dbus_stub.h
    ${CMAKE_BINARY_DIR}/generated/accounts_dbus_common.cpp
    ${CMAKE_BINARY_DIR}/generated/accounts_dbus_common.h
)

ADD_CUSTOM_COMMAND (OUTPUT ${ACCOUNTS_GENERATED_STUB}
                    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/generated/
                    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_BINARY_DIR}/lib/python3.6/site-packages/ 
                            ${GDBUS_CODEGEN} --generate-cpp-code=${CMAKE_BINARY_DIR}/generated/accounts_dbus
                                        --interface-prefix=com.unikylin.
                                        ${ACCOUNTS_INTROSPECTION_XML}
                    DEPENDS ${ACCOUNTS_INTROSPECTION_XML}
                    COMMENT "Generate the stub for the accounts")


SET (USER_INTROSPECTION_XML ${CMAKE_SOURCE_DIR}/plugins/accounts/com.unikylin.Kiran.System.Accounts.User.xml)

SET (USER_GENERATED_STUB
    ${CMAKE_BINARY_DIR}/generated/user_dbus_stub.cpp
    ${CMAKE_BINARY_DIR}/generated/user_dbus_stub.h
    ${CMAKE_BINARY_DIR}/generated/user_dbus_common.cpp
    ${CMAKE_BINARY_DIR}/generated/user_dbus_common.h
)

ADD_CUSTOM_COMMAND (OUTPUT ${USER_GENERATED_STUB}
                    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/generated/
                    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_BINARY_DIR}/lib/python3.6/site-packages/ 
                            ${GDBUS_CODEGEN} --generate-cpp-code=${CMAKE_BINARY_DIR}/generated/user_dbus
                                        --interface-prefix=com.unikylin.
                                        ${USER_INTROSPECTION_XML}
                    DEPENDS ${USER_INTROSPECTION_XML}
                    COMMENT "Generate the stub for the user")



add_library(${TARGET_NAME} SHARED
    ${MOUSE_H_FILES}
    ${MOUSE_CPP_FILES}
    ${ACCOUNTS_GENERATED_STUB}
    ${USER_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/generated
    ${GLIBMM_INCLUDE_DIRS}
    ${GIOMM_INCLUDE_DIRS}
    ${ACCOUNTS_SERVICE_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/third_party/fmt-5.2.0/include)


target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS}
    -L${LIBDIR})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GLIBMM_LIBRARIES}
    ${GIOMM_LIBRARIES}
    ${ACCOUNTS_SERVICE_LIBRARIES}
    kiran-lib
    fmt)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KSD_PLUGIN_DIR})

install(FILES "accounts.ksd-plugin.in" 
        DESTINATION  ${KSD_PLUGIN_DIR}
        RENAME "accounts.ksd-plugin")

install(FILES "com.unikylin.Kiran.System.Accounts.conf"
        DESTINATION ${SYSCONFDIR}/dbus-1/system.d)