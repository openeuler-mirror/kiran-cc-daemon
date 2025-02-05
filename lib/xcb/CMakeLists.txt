set(TARGET_NAME kxcb)

file(GLOB_RECURSE XCB_H_FILES ./*.h)
file(GLOB_RECURSE XCB_CPP_FILES ./*.cpp)

add_library(${TARGET_NAME} STATIC ${XCB_H_FILES} ${XCB_CPP_FILES})

target_include_directories(${TARGET_NAME} PRIVATE ${XCB_INCLUDE_DIRS}
                                                  ${GLIB2_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${XCB_LIBRARIES} ${GLIB2_LIBRARIES} kbase
                         Qt5::X11Extras)
