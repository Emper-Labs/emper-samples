#include <emper/Emper_Engine.h>
#include <OpenGLComputeBackend.h>
#include <SDLOpenGLRenderer.h>
#include <Flock.h>

#include <filesystem>

int main(int argc, char** argv)
{
    if (argc > 0 && argv[0])
        std::filesystem::current_path(
            std::filesystem::absolute(argv[0]).parent_path());

    emper::Simulation simulation;
    simulation.initialize();

    emper::backend::OpenGLComputeBackend computeBackend;
    emper::backend::SDLOpenGLRenderer renderer("Emper Flock", 1280, 720);
    simulation.setRenderer(&renderer);

    auto& world = simulation.world();

    emper::module::FlockConfig config;
    config.mode = emper::interfaces::module::ComputeMode::GPU;
    
            /*
            config.mode = emper::interfaces::module::ComputeMode::CPU;
            CPUSetting
            
                config.boidCount = 100000;

    config.worldWidth  = 1280.0f;
    config.worldHeight = 720.0f;

    config.maxSpeed     = 80.0f;
    config.initialSpeed = 40.0f;
    config.maxForce     = 25.0f;

    config.perceptionRadius = 50.0f;
    config.separationRadius = 30.0f;

    config.separationWeight = 4.0f;
    config.alignmentWeight  = 1.0f;
    config.cohesionWeight   = 1.0f;

    config.maxNeighbours = 64;
    config.teamCount = 3;
            
            */


            /*
            
            GPU setting
            */
    config.boidCount = 100000;

    config.worldWidth  = 1280.0f;
    config.worldHeight = 720.0f;

    config.maxSpeed     = 80.0f;
    config.initialSpeed = 40.0f;
    config.maxForce     = 25.0f;

    config.perceptionRadius = 50.0f;
    config.separationRadius = 30.0f;

    config.separationWeight = 4.0f;
    config.alignmentWeight  = 1.0f;
    config.cohesionWeight   = 1.5f;

    config.maxNeighbours = 64;
    config.teamCount = 3;


    emper::module::Flock flock(world, config, &computeBackend);
    world.addSystem(&flock);

    simulation.start();
    while (simulation.isRunning())
    {
        simulation.tick();
    }

    simulation.shutdown();
    return 0;
}
