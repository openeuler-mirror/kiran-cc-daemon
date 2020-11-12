cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME mouse)

file(GLOB_RECURSE MOUSE_H_FILES ./*.h)
file(GLOB_RECURSE MOUSE_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(MOUSE mouse com.kylinsec. ${CMAKE_SOURCE_DIR}/plugins/inputdevices/mouse/com.kylinsec.Kiran.SessionDaemon.Mouse.xml)


add_library(${TARGET_NAME} SHARED
    ${MOUSE_H_FILES}
    ${MOUSE_CPP_FILES}
    ${MOUSE_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_BINARY_DIR}/generated
    ${GDKMM3_INCLUDE_DIRS})


target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GDKMM3_LIBRARIES}
    inputdevices-common
    lib-base)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/session)
