
cmake_minimum_required(VERSION 3.2)

project(kiran-cc-daemon VERSION 2.0)

option(build-session-daemon "Build session daemon" ON)
option(build-system-daemon "Build system daemon" ON)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(KSD REQUIRED)

find_package(PkgConfig REQUIRED)

pkg_search_module(GLIBMM REQUIRED glibmm-2.4)
pkg_search_module(GIOMM REQUIRED giomm-2.4)
pkg_search_module(GTHREAD REQUIRED gthread-2.0)
pkg_search_module(GMODULE REQUIRED gmodule-2.0)
pkg_search_module(GIOUNIX REQUIRED gio-unix-2.0)
pkg_search_module(LIBXML2 REQUIRED libxml++-2.6)
pkg_search_module(ZLOG REQUIRED zlog)
pkg_search_module(JSONCPP REQUIRED jsoncpp)
pkg_search_module(GTEST REQUIRED gtest)

if(build-session-daemon)
    pkg_search_module(GDKMM3 REQUIRED gdkmm-3.0)
    pkg_search_module(GTKMM3 REQUIRED gtkmm-3.0)
    pkg_search_module(X11 REQUIRED x11)
    pkg_search_module(XEXT REQUIRED xext)
    pkg_search_module(XERCES REQUIRED xerces-c)
    pkg_search_module(FONTCONFIG REQUIRED fontconfig)
    pkg_search_module(CAIROMM_XLIB REQUIRED cairomm-xlib-1.0)
    pkg_search_module(UPOWER_GLIB REQUIRED upower-glib)
    pkg_search_module(LIBNOTIFY REQUIRED libnotify)
endif()

if(build-system-daemon)
    pkg_search_module(SYSTEMD REQUIRED systemd)
endif()

find_program(GDBUS-CODEGEN-GLIBMM-FOUND gdbus-codegen-glibmm3 PATHS ${CMAKE_BINARY_DIR}/bin/)


SET_COMMON_COMPILER_FLAGS()


if (NOT GDBUS-CODEGEN-GLIBMM-FOUND)

    find_program(PYTHON3-FOUND python3)

    if (!PYTHON3-FOUND)
        message(FATAL_ERROR "not found python3.")
    endif()

    execute_process(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/lib/python3.6/site-packages/
                    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_BINARY_DIR}/lib/python3.6/site-packages/
                    python3 ./setup.py install --prefix=${CMAKE_BINARY_DIR}/
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/third_party/gdbus-codegen-glibmm/
                    RESULT_VARIABLE GDBUS_INSTALL_RESULT)    

    if (GDBUS_INSTALL_RESULT EQUAL 0)
        SET (GDBUS_CODEGEN ${CMAKE_BINARY_DIR}/bin/gdbus-codegen-glibmm3)
    else()
        message(FATAL_ERROR "install gdbus-codegen-glibmm3 fail.")
    endif()
else()
    SET (GDBUS_CODEGEN ${GDBUS-CODEGEN-GLIBMM-FOUND})
endif()

message("gdbus-codegen: ${GDBUS_CODEGEN}")

add_subdirectory(po)
add_subdirectory(data)
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(plugins)
add_subdirectory(test)

add_subdirectory("${PROJECT_SOURCE_DIR}/third_party/fmt-5.2.0" "third_party/fmt-5.2.0")

