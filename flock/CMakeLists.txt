add_executable(flock src/main.cpp)
target_link_libraries(flock PRIVATE
    emper-engine
    emper-backend-sdl3
    emper-module-flock
)