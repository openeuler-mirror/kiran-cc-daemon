cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME appearance)

file(GLOB_RECURSE APPEARANCE_H_FILES ./*.h)
file(GLOB_RECURSE APPEARANCE_CPP_FILES ./*.cpp)

GEN_DBUS_STUB(APPEARANCE appearance com.kylinsec. ${CMAKE_SOURCE_DIR}/plugins/appearance/com.kylinsec.Kiran.SessionDaemon.Appearance.xml)


add_library(${TARGET_NAME} SHARED
    ${APPEARANCE_H_FILES}
    ${APPEARANCE_CPP_FILES}
    ${APPEARANCE_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_BINARY_DIR}/generated
    ${GTKMM3_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS})

target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GTKMM3_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    lib-base)

install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/session)
