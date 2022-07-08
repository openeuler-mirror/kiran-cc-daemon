cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME network)

file(GLOB_RECURSE NETWORK_H_FILES ./*.h)
file(GLOB_RECURSE NETWORK_CPP_FILES ./*.cpp)

gen_dbus_stub(
  NETWORK_PROXY
  network_proxy
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/network/com.kylinsec.Kiran.SessionDaemon.Network.Proxy.xml
)

add_library(${TARGET_NAME} SHARED ${NETWORK_H_FILES} ${NETWORK_CPP_FILES}
                                  ${NETWORK_PROXY_GENERATED_STUB})

set_common_shared_linker_flags()

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_BINARY_DIR}/generated
                                                 ${JSONCPP_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${JSONCPP_LIBRARIES} lib-base)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCC_PLUGIN_DIR}/session)
