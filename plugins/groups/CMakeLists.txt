set(TARGET_NAME groups)

file(GLOB_RECURSE CORE_H_FILES ./*.h)
file(GLOB_RECURSE CORE_CPP_FILES ./*.cpp)

gen_dbus_stub(
  GROUPS
  groups
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/groups/com.kylinsec.Kiran.SystemDaemon.Groups.xml
)
gen_dbus_stub(
  GROUP
  group
  com.kylinsec.
  ${CMAKE_SOURCE_DIR}/plugins/groups/com.kylinsec.Kiran.SystemDaemon.Groups.Group.xml
)

add_library(
  ${TARGET_NAME} SHARED ${CORE_H_FILES} ${CORE_CPP_FILES}
                        ${GROUPS_GENERATED_STUB} ${GROUP_GENERATED_STUB})

set_common_shared_linker_flags()

target_include_directories(
  ${TARGET_NAME} PUBLIC ${PROJECT_BINARY_DIR}/generated ${GIOUNIX_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${KSD_LINKER_FLAGS} ${GIOUNIX_LIBRARIES} lib-base
                         lib-dbus)

install(TARGETS ${TARGET_NAME} DESTINATION ${KCC_PLUGIN_DIR}/system)
