cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME display)

file(GLOB_RECURSE DISPLAY_H_FILES ./*.h)
file(GLOB_RECURSE DISPLAY_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(DISPLAY display com.kylinsec. ${CMAKE_SOURCE_DIR}/plugins/display/com.kylinsec.Kiran.SessionDaemon.Display.xml)
GEN_DBUS_STUB(MONITOR monitor com.kylinsec. ${CMAKE_SOURCE_DIR}/plugins/display/com.kylinsec.Kiran.SessionDaemon.Display.Monitor.xml)

find_program(XSDCXX NAMES "xsdcxx" REQUIRED)


set(DISPLAY_OUTPUT ${PROJECT_BINARY_DIR}/plugins/display/display.hxx
                    ${PROJECT_BINARY_DIR}/plugins/display/display.cxx)

add_custom_command(OUTPUT ${DISPLAY_OUTPUT}
                   COMMAND ${XSDCXX} cxx-tree --std c++11 
                                              --namespace-map =Kiran 
                                              --type-naming ucc
                                              --generate-serialization
                                              ${PROJECT_SOURCE_DIR}/plugins/display/display.xsd
                   COMMENT "generate the c++ file by display.xml")

add_library(${TARGET_NAME} SHARED
    ${DISPLAY_OUTPUT}
    ${DISPLAY_H_FILES}
    ${DISPLAY_CPP_FILES}
    ${DISPLAY_GENERATED_STUB}
    ${MONITOR_GENERATED_STUB})


SET_COMMON_SHARED_LINKER_FLAGS()


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_BINARY_DIR}/generated
    ${GDKMM3_INCLUDE_DIRS}
    ${XERCES_INCLUDE_DIRS})


target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_LINKER_FLAGS})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GDKMM3_LIBRARIES}
    ${XERCES_LIBRARIES}
    lib-base)


install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_PLUGIN_DIR}/session)



