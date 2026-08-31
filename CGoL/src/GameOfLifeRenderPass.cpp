#include "GameOfLifeRenderPass.h"

namespace emper::sample
{

GameOfLifeRenderPass::GameOfLifeRenderPass(
    DataSource dataSource)
    : dataSource_(std::move(dataSource))
{
}

GameOfLifeRenderPass::~GameOfLifeRenderPass() = default;

void GameOfLifeRenderPass::render(
    interfaces::render_pass::RenderPassContext& context)
{
    auto& renderer = context.renderer;

    if (!dataSource_)
        return;

    const auto data = dataSource_();

    if (data.width == 0 || data.height == 0 || data.aliveCells.empty())
        return;

    const float cellWidth =
        static_cast<float>(renderer.windowWidth()) /
        static_cast<float>(data.width);

    const float cellHeight =
        static_cast<float>(renderer.windowHeight()) /
        static_cast<float>(data.height);

    const bool subpixel =
        cellWidth < 1.0f ||
        cellHeight < 1.0f;

    // Iterate only the live cells (O(live cells)), exactly as the original
    // per-backend render loops did. We never scan the full grid here.
    for (const auto cell : data.aliveCells)
    {
        const float screenX =
            static_cast<float>(cell.x) * cellWidth;

        const float screenY =
            static_cast<float>(cell.y) * cellHeight;

        if (subpixel)
        {
            renderer.drawPoint(
                screenX,
                screenY,
                0xFFFFFFFF
            );
        }
        else
        {
            renderer.drawRect(
                screenX,
                screenY,
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
        20.0f
    );
}

} // namespace emper::sample