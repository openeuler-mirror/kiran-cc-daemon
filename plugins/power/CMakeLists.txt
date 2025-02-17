set(TARGET_NAME power)

file(GLOB_RECURSE POWER_H_FILES ./*.h)
file(GLOB_RECURSE POWER_CPP_FILES ./*.cpp)

# Filter tools directory list(FILTER POWER_H_FILES EXCLUDE REGEX .*tools/.*)
# list(FILTER POWER_CPP_FILES EXCLUDE REGEX .*tools/.*)
list(FILTER POWER_H_FILES EXCLUDE REGEX ".*tools/.*")
list(FILTER POWER_CPP_FILES EXCLUDE REGEX ".*tools/.*")

qt5_add_dbus_adaptor(
  POWER_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.SessionDaemon.Power.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/power-manager.h Kiran::PowerManager)

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts"
)

execute_process(COMMAND lupdate-qt5 -no-obsolete ${POWER_H_FILES}
                        ${POWER_CPP_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

add_library(${TARGET_NAME} SHARED ${POWER_H_FILES} ${POWER_CPP_FILES}
                                  ${POWER_SRCS} ${QM_FILES})

if(${KF5Screen_VERSION} VERSION_GREATER_EQUAL "5.27.0")
  target_compile_definitions(${TARGET_NAME} PRIVATE WITH_DPMS_KDE)
  target_link_libraries(${TARGET_NAME} PRIVATE KF5::ScreenDpms)
endif()

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_BINARY_DIR}
          ${CMAKE_CURRENT_BINARY_DIR}
          ${GLIB2_INCLUDE_DIRS}
          ${LIBNOTIFY_INCLUDE_DIRS}
          ${XCB_INCLUDE_DIRS}
          ${XCB_RANDR_INCLUDE_DIRS}
          ${XCB_DPMS_INCLUDE_DIRS}
          ${GSETTINGS_QT_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${XCB_LIBRARIES}
          ${XCB_RANDR_LIBRARIES}
          ${XCB_DPMS_LIBRARIES}
          ${LIBNOTIFY_LIBRARIES}
          ${GSETTINGS_QT_LIBRARIES}
          kbase
          kxcb
          Qt5::X11Extras
          KF5::GlobalAccel
          KF5::XmlGui)

install(FILES ${QM_FILES} DESTINATION ${KCD_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/session)

add_subdirectory(tools)
