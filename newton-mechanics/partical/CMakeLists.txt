add_executable(newton-mechanics-particle
    src/main.cpp
)

target_link_libraries(newton-mechanics-particle
    PRIVATE
        emper-engine
        emper-module-newton-mechanics
)