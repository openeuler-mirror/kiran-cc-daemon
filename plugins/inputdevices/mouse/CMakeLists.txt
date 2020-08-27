cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME mouse)

file(GLOB_RECURSE MOUSE_H_FILES ./*.h)
file(GLOB_RECURSE MOUSE_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(MOUSE mouse ${CMAKE_SOURCE_DIR}/plugins/inputdevices/mouse/com.unikylin.Kiran.SessionDaemon.Mouse.xml)


add_library(${TARGET_NAME} SHARED
    ${MOUSE_H_FILES}
    ${MOUSE_CPP_FILES}
    ${MOUSE_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/generated
    ${GDKMM3_INCLUDE_DIRS})


target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS}
    -L${LIBDIR})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GDKMM3_LIBRARIES}
    inputdevices-common
    fmt)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/session)

install(FILES "mouse.ksd-plugin.in" 
        DESTINATION  ${KCC_PLUGIN_DIR}/session
        RENAME "mouse.ksd-plugin")