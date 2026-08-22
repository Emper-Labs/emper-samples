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
#include <chrono>

constexpr std::size_t Width  = 2048;
constexpr std::size_t Height = 2048;

#define USE_RENDERER

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

    // pattern =
    // emper::module::cgol::loadRLE(
    //     "assets/patterns/gosperglidergun.rle"
    // );


    game.load(pattern,0,0);


    //game.randomize(0.20f);
    //game.load(pattern);

    world.addSystem(&game);

#ifdef USE_RENDERER
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

#endif

    simulation.start();

    using Clock = std::chrono::steady_clock;

    auto lastTime = Clock::now();
    std::size_t frames = 0;

    while (simulation.isRunning())
    {
#ifdef USE_RENDERER
        if (!renderer.processEvents())
            break;
#endif
        simulation.tick(0.1);

        ++frames;

        const auto now = Clock::now();

        const double elapsed =
            std::chrono::duration<double>(
                now - lastTime
            ).count();

        if (elapsed >= 1.0)
        {
            const double fps =
                static_cast<double>(frames) / elapsed;

            std::cout
                << "FPS: "
                << fps
                << '\n';

            frames = 0;
            lastTime = now;
        }
    }

    simulation.shutdown();
    return 0;
}