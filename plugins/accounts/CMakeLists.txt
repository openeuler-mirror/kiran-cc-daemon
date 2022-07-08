cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME accounts)

file(GLOB_RECURSE CORE_H_FILES ./*.h)
file(GLOB_RECURSE CORE_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(ACCOUNTS accounts com.kylinsec. ${CMAKE_SOURCE_DIR}/plugins/accounts/com.kylinsec.Kiran.SystemDaemon.Accounts.xml)
GEN_DBUS_STUB(USER user com.kylinsec. ${CMAKE_SOURCE_DIR}/plugins/accounts/com.kylinsec.Kiran.SystemDaemon.Accounts.User.xml)


add_library(${TARGET_NAME} SHARED
    ${CORE_H_FILES}
    ${CORE_CPP_FILES}
    ${ACCOUNTS_GENERATED_STUB}
    ${USER_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_BINARY_DIR}/generated
    ${GIOUNIX_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS})


target_link_libraries(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS}
    ${GIOUNIX_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    lib-base
    lib-dbus)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/system)



