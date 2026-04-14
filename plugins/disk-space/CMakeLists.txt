set(TARGET_NAME disk-space)

file(GLOB_RECURSE DSH_FILES ./*.h ./*.cpp)

set(TS_FILES "${PROJECT_SOURCE_DIR}/translations/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts")

execute_process(COMMAND lupdate-qt5 -no-obsolete ${DSH_FILES} -ts ${TS_FILES})

qt5_add_translation(QM_FILES ${TS_FILES})

add_library(${TARGET_NAME} SHARED ${DSH_FILES} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_BINARY_DIR}
          ${GIO_UNIX_INCLUDE_DIRS}
          ${GLIB2_INCLUDE_DIRS}
          ${GSETTINGS_QT_INCLUDE_DIRS}
          ${LIBNOTIFY_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${GIO_UNIX_LIBRARIES}
          ${GLIB2_LIBRARIES}
          ${GOBJECT2_LIBRARIES}
          ${GSETTINGS_QT_LIBRARIES}
          ${LIBNOTIFY_LIBRARIES}
          kbase
          Qt5::Widgets)

install(FILES ${QM_FILES} DESTINATION ${KCD_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/session)
