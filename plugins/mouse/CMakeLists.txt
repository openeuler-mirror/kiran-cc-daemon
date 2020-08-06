cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME mouse)

file(GLOB_RECURSE MOUSE_H_FILES ./*.h)
file(GLOB_RECURSE MOUSE_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(MOUSE mouse ${CMAKE_SOURCE_DIR}/plugins/mouse/com.unikylin.Kiran.SessionDaemon.Mouse.xml)


add_library(${TARGET_NAME} SHARED
    ${MOUSE_H_FILES}
    ${MOUSE_CPP_FILES}
    ${MOUSE_GENERATED_STUB})


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
        DESTINATION  ${KSD_PLUGIN_DIR})

install(FILES "mouse.ksd-plugin.in" 
        DESTINATION  ${KSD_PLUGIN_DIR}
        RENAME "mouse.ksd-plugin")