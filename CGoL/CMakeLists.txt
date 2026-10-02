add_executable(CGoL
    src/main.cpp
    src/GameOfLifeRenderPass.cpp
)


target_link_libraries(
        CGoL PRIVATE 
        emper-engine 
        emper-backend-opengl 
        emper-module-CGoL
        )

add_custom_command(
    TARGET CGoL
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E remove_directory
            "$<TARGET_FILE_DIR:CGoL>/assets"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:CGoL>/assets"

    # Module-owned compute shader used by GameOfLifeGPU at runtime (the
    # shader is compiled from disk, so it must be reachable from the binary
    # next to the sample's pattern assets).
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/../../emper-modules/CGoL/assets/shaders"
            "$<TARGET_FILE_DIR:CGoL>/assets/shaders"

    COMMENT "Copying CGoL assets..."
)