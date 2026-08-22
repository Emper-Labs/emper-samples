#include <emper/Emper_Engine.h>
#include <emper/interfaces/backend/IRenderer.h>
#include <emper/interfaces/module/ISystem.h>

#include <SDLOpenGLRenderer.h>
#include <CGoL.h>

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <string>

constexpr std::size_t Width  = 2048;
constexpr std::size_t Height = 2048;

using namespace emper::module::cgol; 
auto main() -> int
{
    emper::Simulation simulation;
    simulation.initialize();

    auto& world = simulation.world();

    GameOfLife game(Width, Height);
    
    Pattern pattern;

    // pattern.name = "Glider";
    // pattern.width = 3;
    // pattern.height = 3;

    // pattern.cells = {
    //     {1, 0},
    //     {2, 1},
    //     {0, 2},
    //     {1, 2},
    //     {2, 2}
    // };
    
    pattern =
    emper::module::cgol::loadRLE(
        "assets/patterns/turingmachine.rle"
    );

    game.load(pattern);


    //game.randomize(0.20f);
    //game.load(pattern);

    world.addSystem(&game);

    emper::backend::SDLOpenGLRenderer renderer(
        "Emper - CGoL",
        1920,
        1080
    );

    if (!renderer.isValid())
    {
        simulation.shutdown();
        return 1;
    }

    simulation.setRenderer(&renderer);
    simulation.start();

    while (simulation.isRunning())
    {
        if (!renderer.processEvents())
            break;

        simulation.tick();
    }

    simulation.shutdown();
    return 0;
}