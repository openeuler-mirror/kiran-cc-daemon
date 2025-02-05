set(TARGET_NAME kinput)

file(GLOB_RECURSE INPUT_H_FILES ./*.h)
file(GLOB_RECURSE INPUT_CPP_FILES ./*.cpp)

add_library(${TARGET_NAME} STATIC ${INPUT_H_FILES} ${INPUT_CPP_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR} ${XCB_INCLUDE_DIRS}
                         ${XCB_XINPUT_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${KSD_LINKER_FLAGS} ${XCB_LIBRARIES}
                         ${XCB_XINPUT_LIBRARIES} kbase kxcb Qt5::Gui)
