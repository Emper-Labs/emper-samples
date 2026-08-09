add_executable(flock src/main.cpp)

set(FLOCK_SHADER_ASSETS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../emper-modules/flock/assets/shaders/flock_comp.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../emper-modules/flock/assets/shaders/flock_ver.ver
    ${CMAKE_CURRENT_SOURCE_DIR}/../../emper-modules/flock/assets/shaders/flock_frag.frag
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/flock_assets.stamp
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        $<TARGET_FILE_DIR:flock>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/../../emper-modules/flock/assets
        $<TARGET_FILE_DIR:flock>/assets
    COMMAND ${CMAKE_COMMAND} -E touch
        ${CMAKE_CURRENT_BINARY_DIR}/flock_assets.stamp
    DEPENDS ${FLOCK_SHADER_ASSETS}
    VERBATIM
)

add_custom_target(flock_assets DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/flock_assets.stamp
)
add_dependencies(flock flock_assets)

target_link_libraries(flock PRIVATE
    emper-engine
    emper-backend-opengl
    emper-module-flock
)