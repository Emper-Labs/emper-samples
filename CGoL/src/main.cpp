#include <emper/Emper_Engine.h>
#include <emper/interfaces/module/ISystem.h>

#include <SDLOpenGLRenderer.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>


namespace
{

using Cell = std::uint8_t;

constexpr std::size_t Width  = 512;
constexpr std::size_t Height = 512;

constexpr std::size_t CellCount = Width * Height;


class GameOfLife
{
public:

    GameOfLife(std::size_t width, std::size_t height)
        : m_width(width)
        , m_height(height)
        , m_current(width * height, 0)
        , m_next(width * height, 0)
    {
    }


    void randomize(float probability = 0.15f)
    {
        std::mt19937 rng{
            std::random_device{}()
        };

        std::bernoulli_distribution alive(probability);

        for (auto& cell : m_current)
            cell = alive(rng) ? 1 : 0;
    }


    void clear()
    {
        std::fill(
            m_current.begin(),
            m_current.end(),
            0
        );
    }


    void step()
    {
        for (std::size_t y = 0; y < m_height; ++y)
        {
            for (std::size_t x = 0; x < m_width; ++x)
            {
                const std::size_t index =
                    y * m_width + x;

                const int neighbors =
                    countNeighbors(x, y);

                const bool alive =
                    m_current[index] != 0;

                bool nextAlive = false;

                if (alive)
                {
                    // Survival: exactly 2 or 3 neighbors.
                    nextAlive =
                        neighbors == 2 ||
                        neighbors == 3;
                }
                else
                {
                    // Birth: exactly 3 neighbors.
                    nextAlive =
                        neighbors == 3;
                }

                m_next[index] =
                    nextAlive ? 1 : 0;
            }
        }

        m_current.swap(m_next);

        ++m_generation;
    }


    Cell cell(
        std::size_t x,
        std::size_t y
    ) const
    {
        return m_current[
            y * m_width + x
        ];
    }


    std::size_t generation() const
    {
        return m_generation;
    }


    std::size_t aliveCount() const
    {
        std::size_t count = 0;

        for (const auto cell : m_current)
            count += cell != 0;

        return count;
    }


    std::size_t width() const
    {
        return m_width;
    }


    std::size_t height() const
    {
        return m_height;
    }


private:

    int countNeighbors(
        std::size_t x,
        std::size_t y
    ) const
    {
        int count = 0;

        // Toroidal world.
        //
        // This means:
        //
        // left of x=0  -> x=width-1
        // right of last -> x=0
        //
        // Same for Y.

        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                if (dx == 0 && dy == 0)
                    continue;

                const std::size_t nx =
                    (x + m_width + dx) % m_width;

                const std::size_t ny =
                    (y + m_height + dy) % m_height;

                count +=
                    m_current[
                        ny * m_width + nx
                    ];
            }
        }

        return count;
    }


private:

    std::size_t m_width;
    std::size_t m_height;

    // Ping-pong buffers.
    std::vector<Cell> m_current;
    std::vector<Cell> m_next;

    std::size_t m_generation = 0;
};


class GameOfLifeRenderer
{
public:

    explicit GameOfLifeRenderer(
        emper::backend::SDLOpenGLRenderer& renderer
    )
        : m_renderer(renderer)
    {
    }


    void render(
        const GameOfLife& simulation
    )
    {
        const float width =
            static_cast<float>(
                m_renderer.windowWidth()
            );

        const float height =
            static_cast<float>(
                m_renderer.windowHeight()
            );

        const float cellWidth =
            width /
            static_cast<float>(
                simulation.width()
            );

        const float cellHeight =
            height /
            static_cast<float>(
                simulation.height()
            );


        for (std::size_t y = 0;
             y < simulation.height();
             ++y)
        {
            for (std::size_t x = 0;
                 x < simulation.width();
                 ++x)
            {
                if (!simulation.cell(x, y))
                    continue;

                const float px =
                    (static_cast<float>(x) + 0.5f)
                    * cellWidth;

                const float py =
                    (static_cast<float>(y) + 0.5f)
                    * cellHeight;

                m_renderer.drawPoint(
                    px,
                    py,
                    0xFFFFFFFF
                );
            }
        }
    }


private:

    emper::backend::SDLOpenGLRenderer& m_renderer;
};

}

auto main() -> int
{

    emper::Simulation simulation;

    simulation.initialize();

    auto& world = simulation.world();

    (void)world;

    emper::backend::SDLOpenGLRenderer renderer(
        "Emper - Conway's Game of Life",
        1024,
        1024
    );

    if (!renderer.isValid())
    {
        std::cerr
            << "Failed to initialize OpenGL renderer\n";

        simulation.shutdown();

        return 1;
    }


    GameOfLife game(
        Width,
        Height
    );

    game.randomize(0.20f);


    GameOfLifeRenderer gameRenderer(
        renderer
    );


    simulation.start();


    using Clock =
        std::chrono::steady_clock;

    auto previous =
        Clock::now();

    double accumulator = 0.0;

    constexpr double stepTime =
        1.0 / 30.0;


    while (simulation.isRunning())
    {
        if (!renderer.processEvents())
            break;


        const auto now =
            Clock::now();

        const double delta =
            std::chrono::duration<double>(
                now - previous
            ).count();

        previous = now;

        accumulator +=
            std::min(delta, 0.25);

        //fix dt

        while (accumulator >= stepTime)
        {
            game.step();

            accumulator -=
                stepTime;
        }



        //frame
        renderer.beginFrame();

        gameRenderer.render(game);

        renderer.drawText(
            "Conway's Game of Life",
            10.0f,
            10.0f,
            1.0f
        );

        renderer.endFrame();
        
        
        //tick
        simulation.tick();
    }


 
    simulation.shutdown();

    return 0;
}