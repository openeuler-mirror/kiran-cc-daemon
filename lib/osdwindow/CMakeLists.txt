set(TARGET_NAME osdwindow)

file(GLOB_RECURSE OSD_H_FILES ./*.h)
file(GLOB_RECURSE OSD_CPP_FILES ./*.cpp)

add_library(${TARGET_NAME} STATIC ${OSD_H_FILES} ${OSD_CPP_FILES})

target_link_libraries(${TARGET_NAME} PRIVATE kbase Qt5::Widgets)
