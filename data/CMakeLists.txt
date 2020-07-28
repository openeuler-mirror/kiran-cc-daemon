cmake_minimum_required(VERSION 3.0)


find_program(INTLTOOL-MERGE NAMES "intltool-merge" REQUIRED)

file(GLOB IN_FILES ./*.in)

foreach(IN_FILE IN LISTS IN_FILES)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" OUT_FILE ${IN_FILE} )
    execute_process(COMMAND ${INTLTOOL-MERGE} -x ${PROJECT_SOURCE_DIR}/po/ ${IN_FILE} ${PROJECT_BINARY_DIR}/data/${OUT_FILE})

    install(FILES ${PROJECT_BINARY_DIR}/data/${OUT_FILE}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions/)
endforeach()



