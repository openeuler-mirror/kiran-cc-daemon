cmake_minimum_required(VERSION 3.0)


find_program(INTLTOOL-MERGE NAMES "intltool-merge" REQUIRED)
pkg_search_module(SYSTEMD REQUIRED systemd)


file(GLOB POLICY_FILES ./*.policy.in)
file(GLOB SERVICE_FILES ./*.service.in)


foreach(IN_FILE IN LISTS POLICY_FILES)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" OUT_FILE ${IN_FILE})
    execute_process(COMMAND ${INTLTOOL-MERGE} -x ${PROJECT_SOURCE_DIR}/po/ ${IN_FILE} ${PROJECT_BINARY_DIR}/data/${OUT_FILE})

    install(FILES ${PROJECT_BINARY_DIR}/data/${OUT_FILE}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions/)
endforeach()


foreach(IN_FILE IN LISTS SERVICE_FILES)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" OUT_FILE ${IN_FILE})
    configure_file(${IN_FILE} ${PROJECT_BINARY_DIR}/data/${OUT_FILE})
endforeach()


if (SYSTEMD_FOUND)
    pkg_get_variable(SYSTEM_UNIT_DIR systemd systemdsystemunitdir)
    message(STATUS "systemdsystemunitdir: ${SYSTEM_UNIT_DIR}")
    install(FILES ${PROJECT_BINARY_DIR}/data/kiran-system-daemon.service
            DESTINATION ${SYSTEM_UNIT_DIR})
endif()


