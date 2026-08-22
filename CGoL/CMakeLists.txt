add_executable(CGoL src/main.cpp)


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

    COMMENT "Copying CGoL assets..."
)