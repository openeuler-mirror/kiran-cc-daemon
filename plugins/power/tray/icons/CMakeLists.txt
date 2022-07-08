file(GLOB_RECURSE SVG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ksm-*.svg)
install(
  FILES ${SVG_FILES}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/")
