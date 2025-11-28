add_subdirectory(base)
add_subdirectory(dbus)
add_subdirectory(iso)
add_subdirectory(osdwindow)

if(build-session-daemon)
  add_subdirectory(display)
endif()
