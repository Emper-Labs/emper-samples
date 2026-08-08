#include <emper/Emper_Engine.h>
#include <Renderer.h>
#include <UniformGrid.h>

#include <SDL3/SDL.h>

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <omp.h>
#include <random>

namespace
{

constexpr float WindowWidth  = 1280.0f;
constexpr float WindowHeight = 720.0f;

constexpr std::size_t BoidCount = 100000;

// ============================================================
// Physics
// ============================================================
//
// All velocities are pixels / second.
// All accelerations are pixels / second^2.
//

constexpr float MaxSpeed = 100.0f;
constexpr float MaxForce = 40.0f;

constexpr float InitialSpeed = 60.0f;

// ============================================================
// Neighborhood
// ============================================================

constexpr float PerceptionRadius = 30.0f;
constexpr float SeparationRadius  = 30.0f;

// ============================================================
// Steering
// ============================================================

constexpr float SeparationWeight = 5.0f;
constexpr float AlignmentWeight  = 2.0f;
constexpr float CohesionWeight   = 1.0f;

constexpr std::size_t TeamCount = 3;

constexpr emper::u32 TeamColors[TeamCount] = {
    0xFF4D4DFF,
    0x4DFF88FF,
    0x4D8DFFFF
};

const char* const WindowTitle = "Emper Flock";


// ============================================================
// Math
// ============================================================

emper::Vec2 limit(emper::Vec2 v, float max)
{
    const float lenSq =
        v.x * v.x +
        v.y * v.y;

    const float maxSq =
        max * max;

    if (lenSq > maxSq && lenSq > 0.0f)
    {
        const float invLen =
            1.0f / std::sqrt(lenSq);

        const float scale =
            invLen * max;

        v.x *= scale;
        v.y *= scale;
    }

    return v;
}

} // namespace


// ============================================================
// Boid
// ============================================================

struct Boid
{
    emper::Vec2 position;
    emper::Vec2 velocity;
    std::size_t team = 0;

    static void onTick(
        emper::storage::StorageView<Boid>& boids,
        float dt,
        emper::interfaces::backend::IRenderer* renderer);
};


// ============================================================
// Boid Update
// ============================================================

void Boid::onTick(
    emper::storage::StorageView<Boid>& boids,
    float dt,
    emper::interfaces::backend::IRenderer* renderer)
{
    auto pos =
        boids.column<&Boid::position>();

    auto vel =
        boids.column<&Boid::velocity>();

    auto team =
        boids.column<&Boid::team>();

    const std::size_t count =
        boids.size();

    if (count == 0)
        return;


    // ============================================================
    // UPDATE
    // ============================================================

    if (dt > 0.0f && renderer)
    {
        const float fw =
            static_cast<float>(
                renderer->windowWidth());

        const float fh =
            static_cast<float>(
                renderer->windowHeight());

        if (fw <= 0.0f || fh <= 0.0f)
            return;


        // ========================================================
        // Spatial Grid
        // ========================================================

        static emper::module::UniformGrid grid;

        static float gridWidth  = 0.0f;
        static float gridHeight = 0.0f;

        if (gridWidth != fw ||
            gridHeight != fh)
        {
            grid.resize(
                fw,
                fh,
                PerceptionRadius);

            gridWidth  = fw;
            gridHeight = fh;
        }

        grid.rebuild(
            pos.data(),
            count);


        constexpr float separationRadiusSq =
            SeparationRadius *
            SeparationRadius;

        constexpr float epsilonSq =
            0.000001f;

        constexpr float minimumSpeed =
            InitialSpeed * 0.75f;

        constexpr float minimumSpeedSq =
            minimumSpeed * minimumSpeed;

        constexpr float maxSpeedSq =
            MaxSpeed * MaxSpeed;

        constexpr float maxForceSq =
            MaxForce * MaxForce;


        // ========================================================
        // Steering
        // ========================================================
        #pragma omp parallel for
        for (std::size_t i = 0;
             i < count;
             ++i)
        {
            const float px =
                pos[i].x;

            const float py =
                pos[i].y;

            const float vx =
                vel[i].x;

            const float vy =
                vel[i].y;


            float separationX = 0.0f;
            float separationY = 0.0f;

            float alignmentX = 0.0f;
            float alignmentY = 0.0f;

            float cohesionX = 0.0f;
            float cohesionY = 0.0f;

            std::size_t sameTeamNeighbors = 0;


            // ====================================================
            // Neighbor Search
            // ====================================================

            grid.query(
                px,
                py,
                PerceptionRadius,24,
                [&](std::size_t idx,
                    float dx,
                    float dy,
                    float distSq)
                {
                    if (idx == i)
                        return;


                    // =================================================
                    // Separation
                    // =================================================

                    if (distSq < separationRadiusSq &&
                        distSq > epsilonSq)
                    {
                        const float distance =
                            std::sqrt(distSq);

                        const float invDistance =
                            1.0f / distance;


                        // Stronger when closer.
                        const float strength =
                            (SeparationRadius - distance) /
                            SeparationRadius;


                        separationX -=
                            dx *
                            invDistance *
                            strength;

                        separationY -=
                            dy *
                            invDistance *
                            strength;
                    }


                    // =================================================
                    // Alignment (only with same team)
                    // =================================================

                    if (team[i] == team[idx])
                    {
                        alignmentX +=
                            vel[idx].x;

                        alignmentY +=
                            vel[idx].y;


                        // =================================================
                        // Cohesion (only with same team)
                        // =================================================

                        cohesionX +=
                            dx;

                        cohesionY +=
                            dy;

                        ++sameTeamNeighbors;
                    }


                });

            const float invSameTeamNeighbors =
                sameTeamNeighbors > 0
                    ? 1.0f / static_cast<float>(sameTeamNeighbors)
                    : 0.0f;


            // ========================================================
            // SEPARATION
            // ========================================================

            emper::Vec2 separation{
                separationX,
                separationY
            };

            separation =
                limit(
                    separation,
                    MaxForce);


            // ========================================================
            // ALIGNMENT
            // ========================================================

            if (sameTeamNeighbors > 0)
            {
                alignmentX *=
                    invSameTeamNeighbors;

                alignmentY *=
                    invSameTeamNeighbors;


                // Average neighbor velocity
                // becomes desired velocity.

                alignmentX -= vx;
                alignmentY -= vy;
            }


            emper::Vec2 alignment{
                alignmentX,
                alignmentY
            };

            alignment =
                limit(
                    alignment,
                    MaxForce);


            // ========================================================
            // COHESION
            // ========================================================

            cohesionX *=
                invSameTeamNeighbors;

            cohesionY *=
                invSameTeamNeighbors;


            emper::Vec2 cohesion{
                cohesionX,
                cohesionY
            };


            const float cohesionSq =
                cohesion.x * cohesion.x +
                cohesion.y * cohesion.y;

            if (cohesionSq > epsilonSq)
            {
                const float distance =
                    std::sqrt(cohesionSq);

                const float t =
                    std::min(
                        distance / PerceptionRadius,
                        1.0f);

                // Scale desired speed by distance.
                const float desiredSpeed =
                    MaxSpeed * t;

                const float invDistance =
                    1.0f / distance;

                cohesion.x *=
                    invDistance * desiredSpeed;

                cohesion.y *=
                    invDistance * desiredSpeed;

                cohesion.x -= vx;
                cohesion.y -= vy;

                cohesion =
                    limit(cohesion, MaxForce);
            }

            // ========================================================
            // COMBINE STEERING
            // ========================================================

            float accelerationX =
                separation.x *
                    SeparationWeight +

                alignment.x *
                    AlignmentWeight +

                cohesion.x *
                    CohesionWeight;


            float accelerationY =
                separation.y *
                    SeparationWeight +

                alignment.y *
                    AlignmentWeight +

                cohesion.y *
                    CohesionWeight;


            // ========================================================
            // Limit TOTAL acceleration
            // ========================================================

            const float accelerationSq =
                accelerationX * accelerationX +
                accelerationY * accelerationY;


            if (accelerationSq > maxForceSq)
            {
                const float invLength =
                    1.0f /
                    std::sqrt(accelerationSq);

                const float scale =
                    MaxForce *
                    invLength;

                accelerationX *=
                    scale;

                accelerationY *=
                    scale;
            }


            // ========================================================
            // Integrate velocity
            // ========================================================

            float newVX =
                vx +
                accelerationX * dt;

            float newVY =
                vy +
                accelerationY * dt;


            // ========================================================
            // Prevent flock from killing its velocity
            // ========================================================

            const float newVelocitySq =
                newVX * newVX +
                newVY * newVY;


            if (newVelocitySq < minimumSpeedSq)
            {
                if (newVelocitySq > epsilonSq)
                {
                    const float invLength =
                        1.0f /
                        std::sqrt(newVelocitySq);

                    const float scale =
                        minimumSpeed *
                        invLength;

                    newVX *= scale;
                    newVY *= scale;
                }
                else
                {
                    // Completely stopped.
                    // Restore a deterministic direction.

                    const float oldVelocitySq =
                        vx * vx +
                        vy * vy;

                    if (oldVelocitySq > epsilonSq)
                    {
                        const float invOldLength =
                            1.0f /
                            std::sqrt(oldVelocitySq);

                        newVX =
                            vx *
                            invOldLength *
                            minimumSpeed;

                        newVY =
                            vy *
                            invOldLength *
                            minimumSpeed;
                    }
                    else
                    {
                        newVX =
                            InitialSpeed;

                        newVY =
                            0.0f;
                    }
                }
            }


            // ========================================================
            // Maximum velocity
            // ========================================================

            const float finalVelocitySq =
                newVX * newVX +
                newVY * newVY;


            if (finalVelocitySq > maxSpeedSq)
            {
                const float invLength =
                    1.0f /
                    std::sqrt(finalVelocitySq);

                const float scale =
                    MaxSpeed *
                    invLength;

                newVX *=
                    scale;

                newVY *=
                    scale;
            }


            // ========================================================
            // Store velocity
            // ========================================================

            vel[i].x =
                newVX;

            vel[i].y =
                newVY;
        }


        // ========================================================
        // Integrate Positions
        // ========================================================

        for (std::size_t i = 0;
             i < count;
             ++i)
        {
            float x =
                pos[i].x +
                vel[i].x * dt;

            float y =
                pos[i].y +
                vel[i].y * dt;


            // ====================================================
            // Toroidal wrapping
            // ====================================================

            if (x < 0.0f)
                x += fw;
            else if (x >= fw)
                x -= fw;


            if (y < 0.0f)
                y += fh;
            else if (y >= fh)
                y -= fh;


            pos[i].x =
                x;

            pos[i].y =
                y;
        }
    }


    // ============================================================
    // RENDER
    // ============================================================
    if (renderer && dt == 0.0f)
    {
        for (std::size_t i = 0;
             i < count;
             ++i)
        {
            renderer->drawPoint(
                pos[i].x,
                pos[i].y,
                TeamColors[team[i]]);
        }
    }
}

// ============================================================
// Main
// ============================================================

int main()
{
    emper::Simulation simulation;

    simulation.initialize();


    // --------------------------------------------------------
    // Renderer
    // --------------------------------------------------------

    emper::backend::SDL3Renderer renderer(
        WindowTitle,
        static_cast<int>(WindowWidth),
        static_cast<int>(WindowHeight));

    simulation.setRenderer(
        &renderer);


    // --------------------------------------------------------
    // World
    // --------------------------------------------------------

    auto& world =
        simulation.world();


    world.registerType<Boid>()
        .field<&Boid::position>()
        .field<&Boid::velocity>()
        .field<&Boid::team>()
        .onTick(Boid::onTick);


    world.reserve<Boid>(
        BoidCount);


    // --------------------------------------------------------
    // Spawn
    // --------------------------------------------------------

    std::mt19937 rng(42);

    std::uniform_real_distribution<float> posX(
        0.0f,
        WindowWidth);

    std::uniform_real_distribution<float> posY(
        0.0f,
        WindowHeight);

    std::uniform_real_distribution<float> angle(
        0.0f,
        6.28318530718f);


    for (std::size_t i = 0;
         i < BoidCount;
         ++i)
    {
        auto boid =
            world.create<Boid>();


        // Random position.

        boid.set<&Boid::position>({
            posX(rng),
            posY(rng)
        });


        // Random velocity.

        const float a =
            angle(rng);

        boid.set<&Boid::velocity>({
            std::cos(a) * InitialSpeed,
            std::sin(a) * InitialSpeed
        });

        // BoidCount is divisible by TeamCount, so each flock is equal.
        boid.set<&Boid::team>(
            i % TeamCount);
    }


    // --------------------------------------------------------
    // Run
    // --------------------------------------------------------
    //printf("OpenMP threads: %d\n", omp_get_max_threads());

    simulation.start();

    Uint64 lastStats =
        SDL_GetTicks();

    int frames =
        0;


    while (renderer.processEvents())
    {
        float dt =
            renderer.frameDeltaSeconds();

        if (dt > 0.05f)
            dt = 0.05f;


        simulation.tick(dt);

        ++frames;


        const Uint64 now =
            SDL_GetTicks();

        if (now - lastStats >= 1000)
        {
            const float elapsed =
                static_cast<float>(
                    now - lastStats) /
                1000.0f;

            const float fps =
                static_cast<float>(frames) /
                elapsed;

            std::printf(
                "FPS: %.3f | Boids: %zu\n",
                fps,
                BoidCount);

            lastStats = now;
            frames = 0;
        }
    }


    simulation.shutdown();

    return 0;
}
