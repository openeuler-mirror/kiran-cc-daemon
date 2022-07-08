cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME lib-iso)

file(GLOB_RECURSE ISO_H_FILES ./*.h)
file(GLOB_RECURSE ISO_CPP_FILES ./*.cpp)

set(ALL_FILES ${ISO_H_FILES} ${ISO_CPP_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES})

target_include_directories(${TARGET_NAME} PRIVATE ${LIBXML2_INCLUDE_DIRS})

pkg_get_variable(ISO_CODES_PREFIX iso-codes prefix)

if(NOT ISO_CODES_PREFIX)
  set(ISO_CODES_PREFIX "/usr")
endif()

target_compile_definitions(${TARGET_NAME}
                           PRIVATE -DKCC_ISO_CODES_PREFIX="${ISO_CODES_PREFIX}")

target_link_libraries(${TARGET_NAME} PRIVATE ${LIBXML2_LIBRARIES} lib-base)
