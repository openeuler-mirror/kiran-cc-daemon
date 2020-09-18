cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME lib-base)

file(GLOB_RECURSE BASE_CPP_FILES ./*.h)
file(GLOB_RECURSE BASE_CPP_FILES ./*.cpp)

set (ALL_FILES ${BASE_CPP_FILES} ${BASE_CPP_FILES})

add_library(${TARGET_NAME} ${ALL_FILES})


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}
    ${GIOMM_INCLUDE_DIRS} 
    ${PROJECT_SOURCE_DIR}/third_party/fmt-5.2.0/include)


target_compile_options(${TARGET_NAME} PUBLIC
    ${KSD_COMPILER_FLAGS}
    ${GIOMM_COMPILER_FLAGS}
    )


target_link_libraries(${TARGET_NAME} PUBLIC
    ${GIOMM_LIBRARIES}
    fmt)

