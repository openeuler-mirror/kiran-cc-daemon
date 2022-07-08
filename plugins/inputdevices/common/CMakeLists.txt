cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME inputdevices-common)

file(GLOB_RECURSE COMMON_H_FILES ./*.h)
file(GLOB_RECURSE COMMON_CPP_FILES ./*.cpp)

add_library(${TARGET_NAME} STATIC ${COMMON_H_FILES} ${COMMON_CPP_FILES})

target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}
                                                 ${GDKMM3_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${KSD_LINKER_FLAGS}
                                             ${GDKMM3_LIBRARIES} lib-base)
