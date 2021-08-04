cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME lib-display)

file(GLOB_RECURSE DISPLAY_H_FILES ./*.h)
file(GLOB_RECURSE DISPLAY_CPP_FILES ./*.cpp)

add_library(${TARGET_NAME} STATIC ${DISPLAY_H_FILES} ${DISPLAY_CPP_FILES})

target_include_directories(${TARGET_NAME} PUBLIC 
    ${X11_INCLUDE_DIRS}
    ${GDKMM3_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${X11_LIBRARIES}
    ${GDKMM3_LIBRARIES}
    lib-base)

