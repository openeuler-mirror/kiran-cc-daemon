cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME kiran-system-daemon)

file(GLOB_RECURSE CORE_H_FILES ./*.h)
file(GLOB_RECURSE CORE_CPP_FILES ./*.cpp)



set (ALL_FILES ${CORE_H_FILES} ${CORE_CPP_FILES})

add_executable(${TARGET_NAME} ${ALL_FILES})


SET_COMMON_EXE_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${GLIBMM_INCLUDE_DIRS}
    ${GIOMM_INCLUDE_DIRS} 
    ${GMODULE_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/third_party/fmt-5.2.0/include)

target_compile_definitions(${TARGET_NAME} PRIVATE
    -DKSD_PLUGIN_DIR="${KSD_PLUGIN_DIR}"
    -DKSD_PLUGIN_EXT="${KSD_PLUGIN_EXT}"
    -DKIRAN_SYSTEM_DAEMON_LOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale")

target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS}
    -L${LIBDIR})


target_link_libraries(${TARGET_NAME} PRIVATE
    ${GLIBMM_LIBRARIES}
    ${GIOMM_LIBRARIES}
    ${GMODULE_LIBRARIES}
    kiran-lib
    fmt)

install(FILES "${CMAKE_SOURCE_DIR}/src/com.unikylin.Kiran.SystemDaemon.gschema.xml" 
    DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/
)


install(CODE "execute_process(COMMAND glib-compile-schemas \"${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/\")")