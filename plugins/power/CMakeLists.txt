cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME power)

file(GLOB_RECURSE POWER_H_FILES ./*.h)
file(GLOB_RECURSE POWER_CPP_FILES ./*.cpp)

# Filter tools directory
# list(FILTER POWER_H_FILES EXCLUDE REGEX .*tools/.*)
# list(FILTER POWER_CPP_FILES EXCLUDE REGEX .*tools/.*)
list(FILTER POWER_CPP_FILES EXCLUDE REGEX "(.*tools/main.cpp|.*/tray/.*)")

GEN_DBUS_STUB(POWER power com.kylinsec. ${CMAKE_SOURCE_DIR}/plugins/power/com.kylinsec.Kiran.SessionDaemon.Power.xml)


add_library(${TARGET_NAME} SHARED
    ${POWER_H_FILES}
    ${POWER_CPP_FILES}
    ${POWER_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_BINARY_DIR}/generated
    ${GTKMM3_INCLUDE_DIRS}
    ${LIBNOTIFY_INCLUDE_DIRS}
    ${XEXT_INCLUDE_DIRS})


target_link_libraries(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS}
    ${GTKMM3_LIBRARIES}
    ${XEXT_LIBRARIES}
    ${LIBNOTIFY_LIBRARIES}
    lib-base)

install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/session)

add_subdirectory(tools)
add_subdirectory(tray)
