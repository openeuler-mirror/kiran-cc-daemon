cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME clipboard)

file(GLOB_RECURSE CLIPBOARD_H_FILES ./*.h)
file(GLOB_RECURSE CLIPBOARD_CPP_FILES ./*.cpp)

add_library(${TARGET_NAME} SHARED ${CLIPOBARD_H_FILES} ${CLIPBOARD_CPP_FILES})

set_common_shared_linker_flags()

target_include_directories(${TARGET_NAME} PUBLIC ${GDKMM3_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${GDKMM3_LIBRARIES} lib-base)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCC_PLUGIN_DIR}/session)
