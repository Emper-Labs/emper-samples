add_executable(compute
    src/main.cpp
    src/compute.cpp
    src/myapp.cpp
)

set(COMPUTE_SHADER_ASSETS
    assets/shaders/particles_compute.comp
    assets/shaders/particles_ver.ver
    assets/shaders/particles_frag.frag
)

target_link_libraries(compute
    PRIVATE
        emper-engine
        emper-backend-opengl
)

set(COMPUTE_RUNTIME_ASSETS_DIR
    $<TARGET_FILE_DIR:compute>/assets
)

# Keep runtime shader files in sync even when only a shader changes.
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/compute_assets.stamp
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${COMPUTE_RUNTIME_ASSETS_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${COMPUTE_RUNTIME_ASSETS_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:compute>/cache/shaders
    COMMAND ${CMAKE_COMMAND} -E touch
        ${CMAKE_CURRENT_BINARY_DIR}/compute_assets.stamp
    DEPENDS ${COMPUTE_SHADER_ASSETS}
    VERBATIM
)

add_custom_target(compute_assets DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/compute_assets.stamp
)

add_dependencies(compute compute_assets)
