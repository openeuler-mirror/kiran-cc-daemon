
cmake_minimum_required(VERSION 3.2)

if(build-system-daemon)
add_subdirectory(accounts)
add_subdirectory(timedate)
endif()

if(build-session-daemon)
add_subdirectory(inputdevices/common)
add_subdirectory(inputdevices/mouse)
add_subdirectory(inputdevices/touchpad)
add_subdirectory(inputdevices/keyboard)
add_subdirectory(keybinding)
add_subdirectory(xsettings)
endif()
