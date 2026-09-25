#include "GameOfLifeRenderPass.h"

namespace emper::sample
{

Vec2 CGoLCamera::worldToScreen(
    Vec2 world,
    float screenWidth,
    float screenHeight) const
{
    return {
        (world.x - position.x) * zoom + screenWidth * 0.5f,
        (world.y - position.y) * zoom + screenHeight * 0.5f
    };
}

Vec2 CGoLCamera::screenToWorld(
    Vec2 screen,
    float screenWidth,
    float screenHeight) const
{
    return {
        (screen.x - screenWidth * 0.5f) / zoom + position.x,
        (screen.y - screenHeight * 0.5f) / zoom + position.y
    };
}

void CGoLCamera::pan(Vec2 delta)
{
    position.x += delta.x / zoom;
    position.y += delta.y / zoom;
}

void CGoLCamera::zoomAt(
    Vec2 screenPosition,
    float factor,
    float screenWidth,
    float screenHeight)
{
    const Vec2 before = screenToWorld(screenPosition, screenWidth, screenHeight);

    zoom *= factor;

    if (zoom < 0.01f)
        zoom = 0.01f;

    if (zoom > 1000.0f)
        zoom = 1000.0f;

    const Vec2 after = screenToWorld(screenPosition, screenWidth, screenHeight);

    position.x += before.x - after.x;
    position.y += before.y - after.y;
}

GameOfLifeRenderPass::GameOfLifeRenderPass(
    DataSource dataSource)
    : dataSource_(std::move(dataSource)),
      camera_()
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
        std::to_string(data.aliveCells.size()) + " cells, " + std::to_string(data.generation) + " generations",
        10.0f,
        10.0f,
        15.0f
    );
}

} // namespace emper::sample