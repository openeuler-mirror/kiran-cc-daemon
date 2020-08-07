cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME touchpad)

file(GLOB_RECURSE TOUCHPAD_H_FILES ./*.h)
file(GLOB_RECURSE TOUCHPAD_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(TOUCHPAD touchpad ${CMAKE_SOURCE_DIR}/plugins/inputdevices/touchpad/com.unikylin.Kiran.SessionDaemon.TouchPad.xml)


add_library(${TARGET_NAME} SHARED
    ${TOUCHPAD_H_FILES}
    ${TOUCHPAD_CPP_FILES}
    ${TOUCHPAD_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/generated
    ${GLIBMM_INCLUDE_DIRS}
    ${GIOMM_INCLUDE_DIRS}
    ${GDKMM3_INCLUDE_DIRS})


target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS}
    -L${LIBDIR})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GLIBMM_LIBRARIES}
    ${GIOMM_LIBRARIES}
    ${GDKMM3_LIBRARIES}
    fmt)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/session)

install(FILES "touchpad.ksd-plugin.in" 
        DESTINATION  ${KCC_PLUGIN_DIR}/session
        RENAME "touchpad.ksd-plugin")