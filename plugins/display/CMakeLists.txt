set(TARGET_NAME display)

file(GLOB_RECURSE DISPLAY_H_FILES ./*.h)
file(GLOB_RECURSE DISPLAY_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  DISPLAY_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.Display.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/display-manager.h Kiran::DisplayManager)

qt5_add_dbus_adaptor(
  DISPLAY_MONITOR_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.Display.Monitor.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/display-monitor.h
  Kiran::DisplayMonitor)

find_program(XSDCXX NAMES "xsdcxx" REQUIRED)

set_property(SOURCE ${PROJECT_BINARY_DIR}/plugins/display/display.hxx
             PROPERTY SKIP_AUTOGEN ON)
set_property(SOURCE ${PROJECT_BINARY_DIR}/plugins/display/display.cxx
             PROPERTY SKIP_AUTOGEN ON)

set(DISPLAY_OUTPUT ${PROJECT_BINARY_DIR}/plugins/display/display.hxx
                   ${PROJECT_BINARY_DIR}/plugins/display/display.cxx)

add_custom_command(
  OUTPUT ${DISPLAY_OUTPUT}
  COMMAND
    ${XSDCXX} cxx-tree --std c++11 --namespace-map =Kiran --type-naming ucc
    --generate-serialization ${PROJECT_SOURCE_DIR}/plugins/display/display.xsd
  COMMENT "generate the c++ file by display.xml")

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts"
)

execute_process(COMMAND lupdate-qt5 -no-obsolete ${DISPLAY_H_FILES}
                        ${DISPLAY_CPP_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

add_library(
  ${TARGET_NAME} SHARED
  ${DISPLAY_OUTPUT} ${DISPLAY_H_FILES} ${DISPLAY_CPP_FILES} ${DISPLAY_SRCS}
  ${DISPLAY_MONITOR_SRCS} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}
          ${XERCES_INCLUDE_DIRS} ${GSETTINGS_QT_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${XERCES_LIBRARIES} ${GSETTINGS_QT_LIBRARIES} kbase
                         KF5::Screen)

install(FILES ${QM_FILES} DESTINATION ${KCD_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/session)
