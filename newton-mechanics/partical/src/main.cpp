#include "emper/EmperEngine.h"
#include "emper/modules/newton_mechanics/NewtonSystem.h"

#include <SDLOpenGLRenderer.h>

#include "ParticleRenderPass.h"

using namespace emper::simulation;
using namespace emper::modules::newton_mechanics;

auto main() -> int{

    Simulation simulation;

    NewtonSystem newtonSystem(simulation.world());

    newtonSystem.addObject({
    {0.0f, 0.0f, 0.0f}, // position
    {0.0f, 0.0f, 0.0f}, // velocity
    {0.0f, 0.0f, 0.0f}, // acceleration
    1.0f                 // mass
    });

    newtonSystem.addObject({
    {1.0f, 0.0f, 1.0f}, // position (z-offset to show the 3D space)
    {0.2f, 0.0f, 0.3f}, // velocity
    {0.0f, 0.0f, 0.0f}, // acceleration
    1.0f                 // mass
    });

    emper::backend::SDLOpenGLRenderer renderer(
        "Emper - Newton Mechanics (particles) 3D",
        1280,
        720
    );

    if (!renderer.isValid())
    {
        simulation.shutdown();
        return 1;
    }

    simulation.setRenderer(renderer);
    simulation.addSystem(newtonSystem);

    emper::sample::ParticleRenderPass renderPass(
        newtonSystem,
        renderer
    );

    simulation.addRenderPass(renderPass);

    simulation.initialize();
    simulation.start();

    while(simulation.tick()){
        // Simulation is running
    }

    simulation.shutdown();

    return 0;
}