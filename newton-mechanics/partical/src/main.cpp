#include "emper/EmperEngine.h"
#include "emper/modules/newton_mechanics/NewtonSystem.h"

#include <SDLOpenGLRenderer.h>

#include "ParticleRenderPass.h"

#include <random>


using namespace emper::simulation;
using namespace emper::modules::newton_mechanics;
using namespace emper;

auto main() -> int{

    Simulation simulation;


    //static constexpr f32 G = 4.0f * 3.14159265359f * 3.14159265359f; <-- if you want to use the real G, you need to scale down the masses and distances of the planets, otherwise the simulation will be unstable and the planets will fly away
    static constexpr f32 G = 1.0f; // <-- sim
    
    
    NewtonSystem newtonSystem(simulation.world(),G);


    //flung far away
    
    // newtonSystem.addObject({
    // {0.0f, 0.0f, 0.0f}, // position
    // {0.0f, 0.0f, 0.0f}, // velocity
    // {0.0f, 0.0f, 0.0f}, // acceleration
    // 1.0f                 // mass
    // });

    // newtonSystem.addObject({
    // {1.0f, 0.0f, 1.0f}, // position (z-offset to show the 3D space)
    // {0.2f, 0.0f, 0.3f}, // velocity
    // {0.0f, 0.0f, 0.0f}, // acceleration
    // 1.0f                 // mass
    // });
    
    // orbiting particles
    // newtonSystem.addObject({
    // {-1.0f, 0.0f, 0.0f},  // position
    // {0.0f, 0.5f, 0.0f},   // velocity
    // {0.0f, 0.0f, 0.0f},   // acceleration
    // 1.0f
    // });

    // newtonSystem.addObject({
    //     {1.0f, 0.0f, 0.0f},   // position
    //     {0.0f, -0.5f, 0.0f},  // velocity
    //     {0.0f, 0.0f, 0.0f},   // acceleration
    //     1.0f
    // });

    // 3 obj


    // newtonSystem.addObject({
    // {-1.0f, 0.0f, 0.0f},  // position
    // {0.0f, 0.5f, 0.0f},   // velocity
    // {0.0f, 0.0f, 0.0f},   // acceleration
    // 1.0f
    // });

    // newtonSystem.addObject({
    //     {1.0f, 0.0f, 0.0f},   // position
    //     {0.0f, -0.5f, 0.0f},  // velocity
    //     {0.0f, 0.0f, 0.0f},   // acceleration
    //     1.0f
    // });
    
    // newtonSystem.addObject({
    // {0.0f, 3.0f, 0.0f},   // position
    // {-0.4f, 0.0f, 0.0f},  // velocity
    // {0.0f, 0.0f, 0.0f},   // acceleration
    // 1.0f                  // mass
    // });
    // 100 particles

    // std::mt19937 rng(42);

    // std::uniform_real_distribution<f32> position(-10.0f, 10.0f);
    // std::uniform_real_distribution<f32> velocity(-1.0f, 1.0f);

    // for (int i = 0; i < 100; ++i)
    // {
    //     newtonSystem.addObject({
    //         {position(rng), position(rng), position(rng)},
    //         {velocity(rng), velocity(rng), velocity(rng)},
    //         {0.0f, 0.0f, 0.0f},
    //         1.0f
    //     });
    // }
    //
    constexpr f32 PI = 3.14159265359f;

    struct PlanetData
    {
        f32 mass;
        f32 radius;
    };

    const PlanetData planets[] =
    {
        {1.660e-7f, 0.387f},   // Mercury
        {2.447e-6f, 0.723f},   // Venus
        {3.003e-6f, 1.000f},   // Earth
        {3.227e-7f, 1.524f},   // Mars
        {9.545e-4f, 5.204f},   // Jupiter
        {2.857e-4f, 9.54f},    // Saturn
        {4.366e-5f, 19.19f},   // Uranus
        {5.151e-5f, 30.06f}    // Neptune
    };

    constexpr f32 sunMass = 1.0f;

    //constexpr f32 sunMass = 332900.0f;

    // Sun
    newtonSystem.addObject({
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        sunMass
    });



    for (const auto& planet : planets)
    {
        const f32 r = planet.radius;

        // Circular-orbit approximation:
        // v = sqrt(G M / r)
        const f32 orbitalSpeed =
            std::sqrt(G * sunMass / r);

        newtonSystem.addObject({
            {r, 0.0f, 0.0f},
            {0.0f, 0.0f, orbitalSpeed},
            {0.0f, 0.0f, 0.0f},
            planet.mass
        });
    }




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