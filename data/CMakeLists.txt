cmake_minimum_required(VERSION 3.0)


find_program(INTLTOOL-MERGE NAMES "intltool-merge" REQUIRED)

if(build-system-daemon)
pkg_search_module(SYSTEMD REQUIRED systemd)


# policy files
file(GLOB POLICY_IN_FILES ./*.policy.in)

foreach(POLICY_IN_FILE IN LISTS POLICY_IN_FILES)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" POLICY_FILE ${POLICY_IN_FILE})
    execute_process(COMMAND ${INTLTOOL-MERGE} -x ${PROJECT_SOURCE_DIR}/po/ ${POLICY_IN_FILE} ${PROJECT_BINARY_DIR}/data/${POLICY_FILE})

    install(FILES ${PROJECT_BINARY_DIR}/data/${POLICY_FILE}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions/)
endforeach()


# service files
file(GLOB SERVICE_IN_FILES ./*.service.in)

foreach(SERVICE_IN_FILE IN LISTS SERVICE_IN_FILES)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" SERVICE_FILE ${SERVICE_IN_FILE})
    configure_file(${SERVICE_IN_FILE} ${PROJECT_BINARY_DIR}/data/${SERVICE_FILE})
endforeach()

if (SYSTEMD_FOUND)
    pkg_get_variable(SYSTEM_UNIT_DIR systemd systemdsystemunitdir)
    message(STATUS "systemdsystemunitdir: ${SYSTEM_UNIT_DIR}")
    install(FILES ${PROJECT_BINARY_DIR}/data/kiran-system-daemon.service
            DESTINATION ${SYSTEM_UNIT_DIR})
endif()

install(FILES ${PROJECT_BINARY_DIR}/data/com.unikylin.Kiran.SystemDaemon.service
        DESTINATION ${DATADIR}/dbus-1/services)

# conf files
file(GLOB CONF_FILES ./*.conf)

foreach(CONF_FILE IN LISTS CONF_FILES)    
    install(FILES ${CONF_FILE}
            DESTINATION ${SYSCONFDIR}/dbus-1/system.d)
endforeach()
endif()

if(build-session-daemon)
# gschema.xml
file(GLOB GSCHEMA_XML_FILES ./*.gschema.xml)

foreach(GSCHEMA_XML_FILE IN LISTS GSCHEMA_XML_FILES)
    install(FILES ${GSCHEMA_XML_FILE}
            DESTINATION  ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/)    
endforeach()

install(CODE "execute_process(COMMAND glib-compile-schemas \"${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/\")")
endif()
