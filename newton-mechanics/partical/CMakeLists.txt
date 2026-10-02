add_executable(newton-mechanics-particle
    src/main.cpp
    src/ParticleRenderPass.cpp
)

target_link_libraries(newton-mechanics-particle
    PRIVATE
        emper-engine
        emper-backend-opengl
        emper-module-newton-mechanics
)