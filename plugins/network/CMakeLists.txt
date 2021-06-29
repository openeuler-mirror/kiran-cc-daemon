cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME network)

file(GLOB_RECURSE NETWORK_H_FILES ./*.h)
file(GLOB_RECURSE NETWORK_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(NETWORK_PROXY network_proxy com.kylinsec. ${CMAKE_SOURCE_DIR}/plugins/network/com.kylinsec.Kiran.SessionDaemon.Network.Proxy.xml)


add_library(${TARGET_NAME} SHARED
    ${NETWORK_H_FILES}
    ${NETWORK_CPP_FILES}
    ${NETWORK_PROXY_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_BINARY_DIR}/generated)

target_link_libraries(${TARGET_NAME} PRIVATE
    lib-base)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/session)
