add_executable(flock src/main.cpp)
target_link_libraries(flock PRIVATE
    emper-engine
    emper-backend-sdl3
    emper-module-uniform-grid
)

find_package(OpenMP REQUIRED)
target_compile_options(flock PRIVATE ${OpenMP_CXX_FLAGS})
target_link_libraries(flock PRIVATE ${OpenMP_CXX_LIBRARIES})