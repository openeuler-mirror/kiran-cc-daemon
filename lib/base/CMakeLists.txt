set(TARGET_NAME kbase)

file(GLOB_RECURSE BASE_H_FILES ./*.h)
file(GLOB_RECURSE BASE_CPP_FILES ./*.cpp)

set(ALL_FILES ${BASE_H_FILES} ${BASE_CPP_FILES})

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts"
)

execute_process(COMMAND lupdate-qt5 -no-obsolete ${BASE_H_FILES}
                        ${BASE_CPP_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
         ${PROJECT_BINARY_DIR} ${KLOG_QT5_INCLUDE_DIRS} ${FMT_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PUBLIC ${KLOG_QT5_LIBRARIES} ${FMT_LIBRARIES} Qt5::Core
                        Qt5::DBus)

install(FILES ${QM_FILES} DESTINATION ${KCD_INSTALL_TRANSLATIONDIR})
target_link_libraries(${TARGET_NAME} PRIVATE ${CRYPTOPP_LIBRARIES})
