include(GNUInstallDirs)

set(KCD_INSTALL_INCLUDE ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${PROJECT_NAME})
set(KCD_INSTALL_DATADIR ${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME})
set(KCD_INSTALL_TRANSLATIONDIR ${KCD_INSTALL_DATADIR}/translations)
set(KCD_PLUGIN_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/${PROJECT_NAME})

set(enable-plugin-accounts
    "true"
    CACHE STRING "Enable plugin accounts")
set(enable-plugin-greeter
    "true"
    CACHE STRING "Enable plugin greeter")
set(enable-plugin-systeminfo
    "true"
    CACHE STRING "Enable plugin systeminfo")
set(enable-plugin-timedate
    "true"
    CACHE STRING "Enable plugin timedate")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
