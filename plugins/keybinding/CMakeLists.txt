cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME keybinding)

file(GLOB_RECURSE KEYBINDING_H_FILES ./*.h)
file(GLOB_RECURSE KEYBINDING_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(KEYBINDING keybinding ${CMAKE_SOURCE_DIR}/plugins/keybinding/com.unikylin.Kiran.SessionDaemon.Keybinding.xml)


add_library(${TARGET_NAME} SHARED
    ${KEYBINDING_H_FILES}
    ${KEYBINDING_CPP_FILES}
    ${KEYBINDING_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/generated
    ${GDKMM3_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/third_party/fmt-5.2.0/include)


target_compile_definitions(${TARGET_NAME} PRIVATE
    -DKCC_LOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale")

target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS}
    -L${LIBDIR})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GDKMM3_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    kiran-lib
    fmt)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/session)

install(FILES "keybinding.ksd-plugin.in" 
        DESTINATION  ${KCC_PLUGIN_DIR}/session
        RENAME "keybinding.ksd-plugin")


