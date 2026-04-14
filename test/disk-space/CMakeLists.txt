enable_testing()

set(TARGET_NAME kcd-test-disk-space)

add_executable(${TARGET_NAME} test-disk-space-helper.cpp test-disk-space-helper.h)

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Core Qt5::Test)

add_test(NAME test-disk-space COMMAND ${TARGET_NAME})
set_tests_properties(test-disk-space PROPERTIES LABELS "session-daemon")
