cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME inputdevices-common)

file(GLOB_RECURSE COMMON_H_FILES ./*.h)
file(GLOB_RECURSE COMMON_CPP_FILES ./*.cpp)

add_library(${TARGET_NAME} 
    ${COMMON_H_FILES}
    ${COMMON_CPP_FILES})


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${GLIBMM_INCLUDE_DIRS}
    ${GIOMM_INCLUDE_DIRS}
    ${GDKMM3_INCLUDE_DIRS})


target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_COMPILER_FLAGS}
    -L${LIBDIR})

target_link_libraries(${TARGET_NAME} PRIVATE
    ${GLIBMM_LIBRARIES}
    ${GIOMM_LIBRARIES}
    ${GDKMM3_LIBRARIES}
    kiran-lib)