#include <emper/EmperEngine.h>
#include <emper/interfaces/backend/IRenderer.h>
#include <emper/interfaces/module/ISystem.h>

#include <SDLOpenGLRenderer.h>
#include <CGoLCPUScalar.h>
#include <CGoLCPUPacked.h>
#include <CGoLCPUSparse.h>

#include "GameOfLifeRenderPass.h"

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
#define FIXED 0

using namespace emper::module::cgol; 
auto main(int argc, char** argv) -> int
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

    std::string rle = "assets/patterns/gemini.rle";

    if (argc > 1)
        rle = argv[1];

    pattern =
    emper::module::cgol::loadRLE(
        rle
    );

    const std::size_t width = ((pattern.width + 128 + 63) / 64) * 64;
    const std::size_t height = ((pattern.height + 128 + 63) / 64) * 64;

    std::cout << " w:" <<  width << " h:" <<height << " rle:" << rle <<  " cell cnt:"<< pattern.cells.size()<<"\n";



    // pattern =
    // emper::module::cgol::loadRLE(
    //     "assets/patterns/gosperglidergun.rle"
    // );


    // The CGoL module is simulation-only and exposes its state through data().
    // Each backend reports GameOfLifeData::aliveCells (a read-only list of live
    // cells) with the same complexity as its native iteration — Sparse/Packed
    // are O(live cells), Scalar is O(grid). No full-grid densification occurs.
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

    // The simulation runs entirely through the system manager, while
    // visualization is performed by the render pass (which consumes the
    // simulation state via game.data()).
    emper::sample::GameOfLifeRenderPass renderPass(
        [&game]() -> emper::module::cgol::GameOfLifeData
        {
            return game.data();
        }
    );

    simulation.addRenderPass(renderPass);

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

if constexpr (FIXED != 0) {
    simulation.tick(FIXED);
} else {
    simulation.tick();
}

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