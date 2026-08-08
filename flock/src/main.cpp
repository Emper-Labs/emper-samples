#include <emper/Emper_Engine.h>
#include <Renderer.h>
#include <flock.h>

int main()
{
    emper::Simulation simulation;
    simulation.initialize();

    emper::backend::SDL3Renderer renderer("Emper Flock", 1280, 720);
    simulation.setRenderer(&renderer);

    auto& world = simulation.world();

    emper::module::FlockConfig config;
    config.mode = emper::interfaces::module::ComputeMode::CPU;
    config.boidCount = 100'000;

    emper::module::Flock flock(world, config);
    world.addSystem(&flock);

    simulation.start();
    while (simulation.tick())
    {
    }

    simulation.shutdown();
    return 0;
}
