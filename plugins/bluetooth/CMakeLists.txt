cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME bluetooth)

file(GLOB_RECURSE BLUETOOTH_H_FILES ./*.h)
file(GLOB_RECURSE BLUETOOTH_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(BLUETOOTH bluetooth com.kylinsec. ${CMAKE_SOURCE_DIR}/plugins/bluetooth/com.kylinsec.Kiran.SessionDaemon.Bluetooth.xml)
GEN_DBUS_STUB(OBJECT_MANAGER object_manager org.freedesktop. ${CMAKE_SOURCE_DIR}/plugins/bluetooth/org.bluez.ObjectManager.xml)
GEN_DBUS_STUB(BLUEZ_ADAPTER bluez_adapter org. ${CMAKE_SOURCE_DIR}/plugins/bluetooth/org.bluez.Adapter1.xml)
GEN_DBUS_STUB(BLUEZ_DEVICE bluez_device org. ${CMAKE_SOURCE_DIR}/plugins/bluetooth/org.bluez.Device1.xml)
GEN_DBUS_STUB(BLUEZ_AGENT bluez_agent org. ${CMAKE_SOURCE_DIR}/plugins/bluetooth/org.bluez.Agent1.xml)
GEN_DBUS_STUB(BLUEZ_AGENT_MANAGER bluez_agent_manager org. ${CMAKE_SOURCE_DIR}/plugins/bluetooth/org.bluez.AgentManager1.xml)


add_library(${TARGET_NAME} SHARED
    ${BLUETOOTH_H_FILES}
    ${BLUETOOTH_CPP_FILES}
    ${BLUETOOTH_GENERATED_STUB}
    ${OBJECT_MANAGER_GENERATED_STUB}
    ${BLUEZ_ADAPTER_GENERATED_STUB}
    ${BLUEZ_DEVICE_GENERATED_STUB}
    ${BLUEZ_AGENT_GENERATED_STUB}
    ${BLUEZ_AGENT_MANAGER_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_BINARY_DIR}/generated)


target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS})

target_link_libraries(${TARGET_NAME} PRIVATE
    lib-base)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/session)
