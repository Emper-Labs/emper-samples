#include <emper/Emper_Engine.h>
#include <emper/interfaces/module/ISystem.h>
#include <emper/interfaces/backend/IRenderer.h>


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
    : public emper::interfaces::module::ISystem
    , public emper::interfaces::behavior::IRenderable
{
public:

    GameOfLife(
        std::size_t width,
        std::size_t height
    )
        : m_width(width)
        , m_height(height)
        , m_current(width * height, 0)
        , m_next(width * height, 0)
    {
    }


    void tick(emper::f32 dt) override
    {
        m_accumulator += dt;

        constexpr float fixedStep = 1.0f / 30.0f;

        while (m_accumulator >= fixedStep)
        {
            step();

            m_accumulator -= fixedStep;
        }
    }


    void render(
        emper::interfaces::backend::IRenderer& renderer
    ) override
    {
        const float width =
            static_cast<float>(
                renderer.windowWidth()
            );

        const float height =
            static_cast<float>(
                renderer.windowHeight()
            );

        const float cellWidth =
            width /
            static_cast<float>(m_width);

        const float cellHeight =
            height /
            static_cast<float>(m_height);


        for (std::size_t y = 0; y < m_height; ++y)
        {
            for (std::size_t x = 0; x < m_width; ++x)
            {
                if (!m_current[y * m_width + x])
                    continue;

                const float px =
                    (static_cast<float>(x) + 0.5f)
                    * cellWidth;

                const float py =
                    (static_cast<float>(y) + 0.5f)
                    * cellHeight;

                renderer.drawPoint(
                    px,
                    py,
                    0xFFFFFFFF
                );
            }
        }

        renderer.drawText(
            "Conway's Game of Life",
            10.0f,
            10.0f,
            1.0f
        );
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

        m_next.clear();
        m_next.resize(m_width * m_height);

        m_generation = 0;
        m_accumulator = 0.0f;
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

                m_next[index] =
                    alive
                        ? (neighbors == 2 || neighbors == 3)
                        : (neighbors == 3);
            }
        }

        m_current.swap(m_next);

        ++m_generation;
    }


    int countNeighbors(
        std::size_t x,
        std::size_t y
    ) const
    {
        int count = 0;

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

    std::vector<Cell> m_current;
    std::vector<Cell> m_next;

    std::size_t m_generation = 0;

    float m_accumulator = 0.0f;
};

}

auto main() -> int
{
    emper::Simulation simulation;

    simulation.initialize();

    auto& world = simulation.world();

    GameOfLife game(
        Width,
        Height
    );

    game.randomize(0.20f);

    world.addSystem(&game);

    emper::backend::SDLOpenGLRenderer renderer(
        "Emper - Conway's Game of Life",
        1024,
        1024
    );

    simulation.setRenderer(&renderer);

    if (!renderer.isValid())
    {
        std::cerr
            << "Failed to initialize OpenGL renderer\n";

        simulation.shutdown();
        return 1;
    }

    simulation.start();

    while (simulation.isRunning())
    {
        if (!renderer.processEvents())
            break;

        simulation.tick();
    }

    simulation.shutdown();

    return 0;
}