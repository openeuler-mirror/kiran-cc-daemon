set(TARGET_NAME clipboard)

file(GLOB_RECURSE CLIPBOARD_H_FILES ./*.h)
file(GLOB_RECURSE CLIPBOARD_CPP_FILES ./*.cpp)

add_library(${TARGET_NAME} SHARED ${CLIPOBARD_H_FILES} ${CLIPBOARD_CPP_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
                         ${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${TARGET_NAME} PRIVATE kbase Qt5::Gui KF5::GuiAddons)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCD_PLUGIN_DIR}/session)
