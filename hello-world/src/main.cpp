#include <emper/Emper_Engine.h>

#include <chrono>
#include <iostream>

struct Particle
{
    emper::Vec2 position;
    emper::Vec2 velocity;
    emper::f32 mass;
};

struct Other{
    emper::f32 id; 
};

int main()
{
    constexpr std::size_t Count = 10'000'000;

    emper::Simulation simulation;

    simulation.initialize();

    auto& world = simulation.world();

    world.registerType<Particle>()
        .field<&Particle::position>()
        .field<&Particle::velocity>()
        .field<&Particle::mass>();

    world.registerType<Other>()
        .field<&Other::id>();

    simulation.start();

    world.reserve<Particle>(Count);

    //----------------------------------------
    // Benchmark Create
    //----------------------------------------

    std::cout << "(Before Create) count:" << world.objectCount() << std::endl;

    auto createBegin = std::chrono::high_resolution_clock::now();

    world.create<Other>();

    std::cout << "count:" << world.objectCount() << std::endl;




    for (std::size_t i = 0; i < Count; ++i)
    {
        auto particle = world.create<Particle>();

        particle.set<&Particle::position>(
            { static_cast<float>(i), static_cast<float>(i * 2) });

        particle.set<&Particle::velocity>(
            { 1.0f + i * 0.01f, 0.5f });

        particle.set<&Particle::mass>(
            1.0f + i * 0.1f);
    }

    auto createEnd = std::chrono::high_resolution_clock::now();

    std::cout
        << "Create: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(
               createEnd - createBegin)
               .count()
        << " ms\n";

    std::cout
        << "Objects: "
        << world.objectCount()
        << "\n";

    //----------------------------------------
    // Benchmark Update
    //----------------------------------------

    auto storage = world.storage<Particle>();

    auto pos = storage.column<&Particle::position>();
    auto vel = storage.column<&Particle::velocity>();

    auto updateBegin = std::chrono::high_resolution_clock::now();

    for (int frame = 0; frame < 1000; ++frame)
    {
        for (std::size_t i = 0; i < storage.size(); ++i)
        {
            pos[i].x += vel[i].x * 0.01f;
            pos[i].y += vel[i].y * 0.01f;
        }
    }

    auto updateEnd = std::chrono::high_resolution_clock::now();

    std::cout
        << "Update (1000 frames): "
        << std::chrono::duration_cast<std::chrono::milliseconds>(
               updateEnd - updateBegin)
               .count()
        << " ms\n";

    //----------------------------------------
    // Benchmark Destroy
    //----------------------------------------

    auto destroyBegin = std::chrono::high_resolution_clock::now();

    world.clear<Particle>();

    auto destroyEnd = std::chrono::high_resolution_clock::now();

    std::cout
        << "Destroy: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(
               destroyEnd - destroyBegin)
               .count()
        << " ms\n";

    std::cout
        << "Objects: "
        << world.objectCount()
        << "\n";

    simulation.shutdown();

    return 0;
}