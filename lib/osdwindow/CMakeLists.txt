cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME lib-osdwindow)

file(GLOB_RECURSE OSD_H_FILES ./*.h)
file(GLOB_RECURSE OSD_CPP_FILES ./*.cpp)

add_library(${TARGET_NAME} STATIC ${OSD_H_FILES} ${OSD_CPP_FILES})

target_include_directories(${TARGET_NAME} PUBLIC ${X11_INCLUDE_DIRS}
                                                 ${GTKMM3_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${X11_LIBRARIES}
                                             ${GTKMM3_LIBRARIES} lib-base)
