cmake_minimum_required(VERSION 3.2)

enable_testing()

set (TARGET_NAME kiran-cc-test)

file(GLOB_RECURSE TEST_H_FILES ./*.h)
file(GLOB_RECURSE TEST_CPP_FILES ./*.cpp)


GEN_DBUS_STUB(ACCOUNTS accounts com.kylinsec. ${CMAKE_SOURCE_DIR}/plugins/accounts/com.kylinsec.Kiran.SystemDaemon.Accounts.xml)
GEN_DBUS_STUB(USER user com.kylinsec. ${CMAKE_SOURCE_DIR}/plugins/accounts/com.kylinsec.Kiran.SystemDaemon.Accounts.User.xml)

add_executable(${TARGET_NAME}
               ${TEST_H_FILES}
               ${TEST_CPP_FILES}
               ${ACCOUNTS_GENERATED_STUB}
               ${USER_GENERATED_STUB})

target_include_directories(${TARGET_NAME} PUBLIC
                           ${PROJECT_SOURCE_DIR}/include
                           ${PROJECT_BINARY_DIR}/generated
                           ${GTEST_INCLUDE_DIRS})           

target_link_libraries(${TARGET_NAME} PRIVATE
                      ${GTEST_LIBRARIES}
                      lib-base)

add_test(NAME test 
        COMMAND ${TARGET_NAME})