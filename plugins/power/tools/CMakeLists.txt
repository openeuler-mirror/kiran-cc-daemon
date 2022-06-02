cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME kiran-power-backlight-helper)

file(GLOB_RECURSE POWER_BACKLIGHT_HELPER_H_FILES ./*.h)
file(GLOB_RECURSE POWER_BACKLIGHT_HELPER_CPP_FILES ./*.cpp)


add_executable(${TARGET_NAME}
    ${POWER_BACKLIGHT_HELPER_H_FILES}
    ${POWER_BACKLIGHT_HELPER_CPP_FILES})


target_link_libraries(${TARGET_NAME} PRIVATE
    lib-base)

install(TARGETS ${TARGET_NAME}
        DESTINATION  ${KCC_INSTALL_BINDIR}/)
