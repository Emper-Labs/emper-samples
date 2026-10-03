#include "ParticleRenderPass.h"
#include "SDLOpenGLRenderer.h"
#include "emper/modules/newton_mechanics/NewtonSystem.h"
#include "emper/simulation/Simulation.h"

#include <cmath>
#include <random>

using namespace emper;
using namespace emper::simulation;
using namespace emper::modules::newton_mechanics;

namespace
{

constexpr f32 G = 1.0f;

enum class Scenario
{
    Single,
    Binary,
    ThreeBody,
    Random100,
    SolarSystem
};

constexpr Scenario scenario = Scenario::SolarSystem;

// ------------------------------------------------------------
// Single particle
// ------------------------------------------------------------

void addSingle(NewtonSystem& system)
{
    system.addObject({
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        1.0f
    });
}

// ------------------------------------------------------------
// Binary system
// ------------------------------------------------------------

void addBinary(NewtonSystem& system)
{
    system.addObject({
        { -1.0f, 0.0f, 0.0f },
        {  0.0f, 0.5f, 0.0f },
        {  0.0f, 0.0f, 0.0f },
        1.0f
    });

    system.addObject({
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, -0.5f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        1.0f
    });
}

// ------------------------------------------------------------
// Three-body system
// ------------------------------------------------------------

void addThreeBody(NewtonSystem& system)
{
    system.addObject({
        { -1.0f, 0.0f, 0.0f },
        { 0.0f, 0.5f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        1.0f
    });

    system.addObject({
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, -0.5f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        1.0f
    });

    system.addObject({
        { 0.0f, 3.0f, 0.0f },
        { -0.4f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        1.0f
    });
}

// ------------------------------------------------------------
// Random particles
// ------------------------------------------------------------

void addRandom(NewtonSystem& system, int count)
{
    std::mt19937 rng(42);

    std::uniform_real_distribution<f32> position(-10.0f, 10.0f);
    std::uniform_real_distribution<f32> velocity(-1.0f, 1.0f);

    for (int i = 0; i < count; ++i)
    {
        system.addObject({
            {
                position(rng),
                position(rng),
                position(rng)
            },
            {
                velocity(rng),
                velocity(rng),
                velocity(rng)
            },
            { 0.0f, 0.0f, 0.0f },
            1.0f
        });
    }
}

// ------------------------------------------------------------
// Solar system
// ------------------------------------------------------------

struct Planet
{
    f32 mass;
    f32 distance;
};

constexpr Planet planets[] =
{
    { 1.660e-7f,  0.387f }, // Mercury
    { 2.447e-6f,  0.723f }, // Venus
    { 3.003e-6f,  1.000f }, // Earth
    { 3.227e-7f,  1.524f }, // Mars
    { 9.545e-4f,  5.204f }, // Jupiter
    { 2.857e-4f,  9.540f }, // Saturn
    { 4.366e-5f, 19.190f }, // Uranus
    { 5.151e-5f, 30.060f }  // Neptune
};

void addSolarSystem(NewtonSystem& system)
{
    constexpr f32 sunMass = 1.0f;

    system.addObject({
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        sunMass
    });

    for (const auto& planet : planets)
    {
        const f32 speed =
            std::sqrt(G * sunMass / planet.distance);

        system.addObject({
            { planet.distance, 0.0f, 0.0f },
            { 0.0f, 0.0f, speed },
            { 0.0f, 0.0f, 0.0f },
            planet.mass
        });
    }
}

// ------------------------------------------------------------
// Scenario selection
// ------------------------------------------------------------

void setupScenario(NewtonSystem& system)
{
    switch (scenario)
    {
    case Scenario::Single:
        addSingle(system);
        break;

    case Scenario::Binary:
        addBinary(system);
        break;

    case Scenario::ThreeBody:
        addThreeBody(system);
        break;

    case Scenario::Random100:
        addRandom(system, 100);
        break;

    case Scenario::SolarSystem:
        addSolarSystem(system);
        break;
    }
}

} // namespace

auto main() -> int
{
    Simulation simulation;

    NewtonSystem newtonSystem(
        simulation.world(),
        G
    );

    setupScenario(newtonSystem);

    emper::backend::SDLOpenGLRenderer renderer(
        "Emper - Newton Mechanics (particles) 3D",
        1280,
        720
    );

    if (!renderer.isValid())
        return 1;

    simulation.setRenderer(renderer);
    simulation.addSystem(newtonSystem);

    emper::sample::ParticleRenderPass renderPass(
        newtonSystem,
        renderer
    );

    simulation.addRenderPass(renderPass);

    simulation.initialize();
    simulation.start();

    while (simulation.tick())
    {
    }

    simulation.shutdown();

    return 0;
}