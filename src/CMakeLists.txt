cmake_minimum_required(VERSION 3.0)

SET_COMMON_EXE_LINKER_FLAGS()

file(GLOB_RECURSE CORE_H_FILES ./*.h)
file(GLOB_RECURSE CORE_CPP_FILES ./*.cpp)

GEN_DBUS_STUB(CC_DAEMON cc_daemon ${CMAKE_SOURCE_DIR}/src/com.unikylin.Kiran.CCDaemon.xml)


# system daemon
set (TARGET_SYSTEM_DAEMON kiran-system-daemon)


add_executable(${TARGET_SYSTEM_DAEMON} 
    ${CORE_H_FILES}
    ${CORE_CPP_FILES}
    ${CC_DAEMON_GENERATED_STUB})

target_include_directories(${TARGET_SYSTEM_DAEMON} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/generated
    ${GLIBMM_INCLUDE_DIRS}
    ${GIOMM_INCLUDE_DIRS} 
    ${GMODULE_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/third_party/fmt-5.2.0/include)

target_compile_definitions(${TARGET_SYSTEM_DAEMON} PRIVATE
    -DKCC_TYPE="system"
    -DKCC_PLUGIN_DIR="${KCC_PLUGIN_DIR}/system"
    -DKCC_PLUGIN_EXT="${KCC_PLUGIN_EXT}"
    -DKCC_LOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale")

target_compile_options(${TARGET_SYSTEM_DAEMON} PRIVATE
    ${KSD_LINKER_FLAGS}
    -L${LIBDIR})


target_link_libraries(${TARGET_SYSTEM_DAEMON} PRIVATE
    ${GLIBMM_LIBRARIES}
    ${GIOMM_LIBRARIES}
    ${GMODULE_LIBRARIES}
    kiran-lib
    fmt)


install(TARGETS ${TARGET_SYSTEM_DAEMON}
        DESTINATION ${EXECDIR})


# session daemon
set (TARGET_SESSION_DAEMON kiran-session-daemon)

add_executable(${TARGET_SESSION_DAEMON} 
    ${CORE_H_FILES}
    ${CORE_CPP_FILES}
    ${CC_DAEMON_GENERATED_STUB})

target_include_directories(${TARGET_SESSION_DAEMON} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/generated
    ${GLIBMM_INCLUDE_DIRS}
    ${GIOMM_INCLUDE_DIRS} 
    ${GMODULE_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/third_party/fmt-5.2.0/include)

target_compile_definitions(${TARGET_SESSION_DAEMON} PRIVATE
    -DKCC_TYPE="session"
    -DKCC_PLUGIN_DIR="${KCC_PLUGIN_DIR}/session"
    -DKCC_PLUGIN_EXT="${KCC_PLUGIN_EXT}"
    -DKCC_LOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale")

target_compile_options(${TARGET_SESSION_DAEMON} PRIVATE
    ${KSD_LINKER_FLAGS}
    -L${LIBDIR})


target_link_libraries(${TARGET_SESSION_DAEMON} PRIVATE
    ${GLIBMM_LIBRARIES}
    ${GIOMM_LIBRARIES}
    ${GMODULE_LIBRARIES}
    kiran-lib
    fmt)


install(TARGETS ${TARGET_SESSION_DAEMON}
        DESTINATION ${EXECDIR})