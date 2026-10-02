#include <emper/EmperEngine.h>
#include <emper/interfaces/backend/IRenderer.h>
#include <emper/interfaces/module/ISystem.h>

#include <SDLOpenGLRenderer.h>
#include <OpenGLComputeBackend.h>
#include <CGoLCPUScalar.h>
#include <CGoLCPUPacked.h>
#include <CGoLCPUSparse.h>
#include <GameOfLifeGPU.h>

#include "GameOfLifeRenderPass.h"

#include <algorithm>
#include <cstdint>
#include <functional>
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

    std::string rle = "assets/patterns/turingmachine.rle";

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
    // cells). CPU backends iterate only live cells (O(live cells)); the GPU
    // backend steps on the GPU (buffer -> shader -> step -> buffer) and does a
    // full-grid readback on data().

#ifdef USE_RENDERER
    // The OpenGL compute backend needs an active rendering context to create
    // its shaders/buffers, so create the renderer before probing the GPU path.
    emper::backend::SDLOpenGLRenderer renderer(
        "Emper - CGoL (GPU)",
        1920,
        1080
    );

    if (!renderer.isValid())
    {
        simulation.shutdown();
        return 1;
    }

    simulation.setRenderer(renderer);

    // Try the GPU-backed implementation first; fall back to the CPU sparse
    // backend when no GPU (OpenGL 4.3 compute) is available.
    emper::backend::OpenGLComputeBackend computeBackend;

    emper::module::cgol::GameOfLifeGPU gpuGame(
        width,
        height,
        &computeBackend
    );

    // Probe the GPU path: compile the shader and allocate buffers. The method
    // is a void ISystem::initialize() override, so availability is queried
    // afterwards via isAvailable().
    gpuGame.initialize();

    const bool useGpu = gpuGame.isAvailable();

    emper::module::cgol::GameOfLifeCPUSparse cpuGame(width, height);

    if (useGpu)
        std::cout << "backend: GPU (OpenGL compute shade)\n";
    else
        std::cout << "backend: CPU (sparse)\n";

    // Tell the GPU render path the drawable surface dimensions (used by the
    // cgol_ver.ver vertex shader to lay the grid out). CPU mode ignores this.
    if (useGpu)
    {
        gpuGame.synchronizeSurface(
            static_cast<emper::f32>(renderer.windowWidth()),
            static_cast<emper::f32>(renderer.windowHeight()));
    }

    if (useGpu)
        gpuGame.load(pattern, 100, 100);
    else
        cpuGame.load(pattern, 100, 100);

    if (useGpu)
        simulation.addSystem(gpuGame);
    else
        simulation.addSystem(cpuGame);

    // The simulation runs entirely through the system manager, while
    // visualization is performed by the render pass (which consumes the
    // simulation state via the chosen backend's data()).
    std::function<emper::module::cgol::GameOfLifeData()> dataSource;

    if (useGpu)
    {
        dataSource = [&gpuGame]()
        {
            return gpuGame.data();
        };
    }
    else
    {
        dataSource = [&cpuGame]()
        {
            return cpuGame.data();
        };
    }

    emper::sample::GameOfLifeRenderPass renderPass(
        dataSource,
        renderer
    );

    simulation.addRenderPass(renderPass);

#else
    // Headless (no renderer): CPU sparse backend only.
    GameOfLifeCPUSparse game(width, height);

    game.load(pattern, 100, 100);

    //game.randomize(0.20f);
    //game.load(pattern);

    simulation.addSystem(game);

    //world.addSystem(&game); ////old api
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