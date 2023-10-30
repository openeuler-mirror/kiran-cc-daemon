find_program(GLIB_COMPILE_RESOURCE NAMES "glib-compile-resources")

# session-daemon
if(build-session-daemon)
  # images file
  file(GLOB SESSION_IMAGES_FILES ${PROJECT_SOURCE_DIR}/resources/images/*.png)
  install(FILES ${SESSION_IMAGES_FILES}
          DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}/images/)

  # gresource
  set(KSD_GRESOURCE_NAME ksd-gresource)

  add_custom_target(
    ${KSD_GRESOURCE_NAME} ALL
    ${GLIB_COMPILE_RESOURCE}
    ${PROJECT_SOURCE_DIR}/resources/kiran-session-daemon.gresource.xml --target
    kiran-session-daemon.gresource --sourcedir ${PROJECT_SOURCE_DIR}/resources/)

  install(FILES ${PROJECT_BINARY_DIR}/resources/kiran-session-daemon.gresource
          DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}/)
endif()
