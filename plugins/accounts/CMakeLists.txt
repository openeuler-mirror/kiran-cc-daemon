cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME accounts)

file(GLOB_RECURSE MOUSE_H_FILES ./*.h)
file(GLOB_RECURSE MOUSE_CPP_FILES ./*.cpp)

pkg_search_module(ACCOUNTS_SERVICE REQUIRED accountsservice)



GEN_DBUS_STUB(ACCOUNTS accounts ${CMAKE_SOURCE_DIR}/plugins/accounts/com.unikylin.Kiran.SystemDaemon.Accounts.xml)
GEN_DBUS_STUB(USER user ${CMAKE_SOURCE_DIR}/plugins/accounts/com.unikylin.Kiran.SystemDaemon.Accounts.User.xml)


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
    ${GIOUNIX_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/third_party/fmt-5.2.0/include)


target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS}
    -L${LIBDIR})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GLIBMM_LIBRARIES}
    ${GIOMM_LIBRARIES}
    ${ACCOUNTS_SERVICE_LIBRARIES}
    ${GIOUNIX_LIBRARIES}
    kiran-lib
    fmt)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/system)

install(FILES "accounts.ksd-plugin.in" 
        DESTINATION  ${KCC_PLUGIN_DIR}/system
        RENAME "accounts.ksd-plugin")



