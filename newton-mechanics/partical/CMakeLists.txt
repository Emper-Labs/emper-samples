add_executable(classical-mechanics-particle
    src/main.cpp
    src/ParticleRenderPass.cpp
)

target_link_libraries(classical-mechanics-particle
    PRIVATE
        emper-engine
        emper-backend-opengl
        emper-module-classical-mechanics
)