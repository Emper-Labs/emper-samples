#include "myapp.h"

#include "compute.h"

namespace
{

constexpr int WindowWidth = 1920;
constexpr int WindowHeight = 1080;
constexpr emper::u32 ParticleCount = 1000;

}

MyApp::MyApp()
    : renderer_("Emper Compute", WindowWidth, WindowHeight),
      compute_(backend_)
{
}

bool MyApp::initialize()
{
    if (!renderer_.isValid())
        return false;

    if (!backend_.initialize())
    {
        shutdown();
        return false;
    }

    if(!compute_.initialize()){
        shutdown();
        return false;
    }

    graphicsProgram_ = renderer_.createGraphicsProgram(
        "assets/shaders/particles_ver.ver",
        "assets/shaders/particles_frag.frag"
    );

    if (!graphicsProgram_)
    {
        shutdown();
        return false;
    }

    return true;
}

void MyApp::render()
{
    renderer_.beginFrame();
    compute_.run(ParticleCount);

    renderer_.bindProgram(
        graphicsProgram_
    );

    renderer_.bindStorageBuffer(
        0,
        compute_.buffer()
    );

    renderer_.drawPoints(ParticleCount);

    renderer_.endFrame();
}

void MyApp::run()
{
    while (renderer_.processEvents())
    {
        render();
    }
}

void MyApp::shutdown()
{
    if (graphicsProgram_)
    {
        renderer_.destroyProgram(graphicsProgram_);
        graphicsProgram_ = 0;
    }

    compute_.shutdown();
    backend_.shutdown();
}