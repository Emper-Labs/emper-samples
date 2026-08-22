add_executable(CGoL src/main.cpp)


target_link_libraries(CGoL PRIVATE emper-engine emper-backend-opengl)