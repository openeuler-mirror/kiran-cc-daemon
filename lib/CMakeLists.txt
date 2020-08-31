cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME kiran-lib)

file(GLOB_RECURSE LIB_H_FILES ./*.h)
file(GLOB_RECURSE LIB_CPP_FILES ./*.cpp)

set (ALL_FILES ${LIB_H_FILES} ${LIB_CPP_FILES})

add_library(${TARGET_NAME} ${ALL_FILES})


target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${GLIBMM_INCLUDE_DIRS}
    ${GIOMM_INCLUDE_DIRS} 
    ${LIBXML2_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/third_party/fmt-5.2.0/include)

pkg_get_variable(ISO_CODES_PREFIX iso-codes prefix)

if (NOT ISO_CODES_PREFIX)
    set(ISO_CODES_PREFIX "/usr")
endif()

target_compile_definitions(${TARGET_NAME} PRIVATE
    -DKCC_ISO_CODES_PREFIX="${ISO_CODES_PREFIX}")

target_compile_options(${TARGET_NAME} PRIVATE
    ${KSD_COMPILER_FLAGS}
    -L${LIBDIR})


target_link_libraries(${TARGET_NAME} PRIVATE
    ${GLIBMM_LIBRARIES}
    ${GIOMM_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    fmt)

