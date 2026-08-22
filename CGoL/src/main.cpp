#include <emper/Emper_Engine.h>
#include <emper/interfaces/backend/IRenderer.h>
#include <emper/interfaces/module/ISystem.h>

#include <SDLOpenGLRenderer.h>

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <string>

constexpr std::size_t Width  = 2048;
constexpr std::size_t Height = 2048;

namespace emper::module::cgol
{

using Cell = emper::i8;

struct CellCoordinate
{
    emper::i32 x;
    emper::i32 y;
};

struct Pattern
{
    std::string name;

    emper::i32 width  = 0;
    emper::i32 height = 0;

    std::vector<CellCoordinate> cells;
};


Pattern loadRLE(const std::string& path)
{
    std::ifstream file(path);

    if (!file)
    {
        throw std::runtime_error(
            "Failed to open RLE file: " + path
        );
    }

    Pattern pattern;

    std::string line;
    std::string data;

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        if (line[0] == '#')
        {
            if (line.size() > 3 && line[1] == 'N')
                pattern.name = line.substr(3);

            continue;
        }

        // Header:
        // x = 3, y = 3, rule = B3/S23
        if (line.find("x =") != std::string::npos)
        {
            auto parseValue =
                [&line](const char* key) -> emper::i32
            {
                const auto pos = line.find(key);

                if (pos == std::string::npos)
                    return 0;

                auto start =
                    pos + std::string(key).size();

                // Skip whitespace after "x ="
                while (
                    start < line.size() &&
                    std::isspace(
                        static_cast<unsigned char>(
                            line[start]
                        )
                    )
                )
                {
                    ++start;
                }

                auto end = start;

                while (
                    end < line.size() &&
                    std::isdigit(
                        static_cast<unsigned char>(
                            line[end]
                        )
                    )
                )
                {
                    ++end;
                }

                if (start == end)
                    throw std::runtime_error(
                        "Invalid RLE header: " + line
                    );

                return static_cast<emper::i32>(
                    std::stoi(
                        line.substr(
                            start,
                            end - start
                        )
                    )
                );
            };

            pattern.width  = parseValue("x =");
            pattern.height = parseValue("y =");

            continue;
        }

        data += line;
    }

    // Decode RLE
    emper::i32 x = 0;
    emper::i32 y = 0;

    std::size_t i = 0;

    while (i < data.size())
    {
        emper::i32 count = 0;

        while (
            i < data.size() &&
            std::isdigit(
                static_cast<unsigned char>(data[i])
            )
        )
        {
            count =
                count * 10 +
                static_cast<emper::i32>(
                    data[i] - '0'
                );

            ++i;
        }

        if (count == 0)
            count = 1;

        if (i >= data.size())
            break;

        const char token = data[i++];

        switch (token)
        {
            case 'o':
            {
                for (emper::i32 n = 0; n < count; ++n)
                {
                    pattern.cells.push_back({
                        x + n,
                        y
                    });
                }

                x += count;
                break;
            }

            case 'b':
            {
                x += count;
                break;
            }

            case '$':
            {
                y += count;
                x = 0;
                break;
            }

            case '!':
                return pattern;

            default:
                throw std::runtime_error(
                    "Invalid RLE token '" +
                    std::string(1, token) +
                    "' in: " +
                    path
                );
        }
    }

    return pattern;
}

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
        , m_current(width * height)
        , m_next(width * height)
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
        const float cellWidth =
            static_cast<float>(renderer.windowWidth()) /
            static_cast<float>(m_width);

        const float cellHeight =
            static_cast<float>(renderer.windowHeight()) /
            static_cast<float>(m_height);

        for (std::size_t y = 0; y < m_height; ++y)
        {
            for (std::size_t x = 0; x < m_width; ++x)
            {
                if (!m_current[y * m_width + x])
                    continue;

                renderer.drawRect(
                    static_cast<float>(x) * cellWidth,
                    static_cast<float>(y) * cellHeight,
                    cellWidth,
                    cellHeight,
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
        std::mt19937 rng{std::random_device{}()};
        std::bernoulli_distribution alive(probability);

        for (auto& cell : m_current)
            cell = alive(rng);
    }


    void load(
    const Pattern& pattern,
    emper::i32 offsetX = 0,
    emper::i32 offsetY = 0
    )
    {
    for (const auto& cell : pattern.cells)
    {
        const auto x =
            cell.x + offsetX;

        const auto y =
            cell.y + offsetY;

        if (x < 0 || y < 0)
            continue;

        if (
            x >= static_cast<emper::i32>(m_width) ||
            y >= static_cast<emper::i32>(m_height)
        )
            continue;

        m_current[
            static_cast<std::size_t>(y) * m_width +
            static_cast<std::size_t>(x)
        ] = 1;
    }
    }

    void clear()
    {
        std::fill(m_current.begin(), m_current.end(), 0);
        std::fill(m_next.begin(), m_next.end(), 0);

        m_generation = 0;
        m_accumulator = 0.0f;
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
                        ? neighbors == 2 || neighbors == 3
                        : neighbors == 3;
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

                const auto nx =
                    (x + m_width + dx) % m_width;

                const auto ny =
                    (y + m_height + dy) % m_height;

                count +=
                    m_current[ny * m_width + nx];
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


using namespace emper::module::cgol; 
auto main() -> int
{
    emper::Simulation simulation;
    simulation.initialize();

    auto& world = simulation.world();

    GameOfLife game(Width, Height);
    
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
    
    pattern =
    emper::module::cgol::loadRLE(
        "assets/patterns/turingmachine.rle"
    );

    game.load(pattern);


    //game.randomize(0.20f);
    //game.load(pattern);

    world.addSystem(&game);

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

    simulation.setRenderer(&renderer);
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