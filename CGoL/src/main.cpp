#include <emper/Emper_Engine.h>
#include <emper/interfaces/backend/IRenderer.h>
#include <emper/interfaces/module/ISystem.h>

#include <SDLOpenGLRenderer.h>
#include <CGoLCPUScalar.h>
#include <CGoLCPUPacked.h>
#include <CGoLCPUSparse.h>

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <string>
#include <chrono>
#include <iostream>

#define USE_RENDERER
//#define FIXED

using namespace emper::module::cgol; 
auto main() -> int
{
    emper::simulation::Simulation simulation;
    simulation.initialize();

    auto& world = simulation.world();


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

    std::string rle = "assets/patterns/universalturingmachine.rle";

    pattern =
    emper::module::cgol::loadRLE(
        rle
    );

    const std::size_t width = ((pattern.width + 128 + 63) / 64) * 64;
    const std::size_t height = ((pattern.height + 128 + 63) / 64) * 64;

    std::cout << " w:" <<  width << " h:" <<height << " rle:" << rle << "\n";



    // pattern =
    // emper::module::cgol::loadRLE(
    //     "assets/patterns/gosperglidergun.rle"
    // );


    //GameOfLifeCPUScalar game(width, height);
    //GameOfLifeCPUPacked game(width, height);
    GameOfLifeCPUSparse game(width, height);

    game.load(pattern,100,100);


    //game.randomize(0.20f);
    //game.load(pattern);

    simulation.addSystem(game);

    //world.addSystem(&game); ////old api

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

    simulation.setRenderer(renderer);

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

#ifdef FIXED
        simulation.tick(1);
#else

 simulation.tick();
#endif


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