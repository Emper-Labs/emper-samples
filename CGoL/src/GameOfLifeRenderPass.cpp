#include "GameOfLifeRenderPass.h"

#include <SDL3/SDL.h>

#include <cmath>
#include <algorithm>

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
    DataSource dataSource,
    interfaces::backend::IRenderer& renderer)
    : dataSource_(std::move(dataSource)),
      camera_(),
      renderer_(renderer)
{
    // Register as a consumer of the renderer's native (SDL) events so the
    // mouse can drive the camera: left-drag to pan, wheel to zoom.
    auto* eventSource =
        dynamic_cast<interfaces::behavior::INativeEventSource*>(
            &renderer_);

    if (eventSource)
    {
        eventSource_ = eventSource;

        // capture `this` in the member callback so camera_ state is updated
        eventSource_->setEventCallback(
            [this](const void* nativeEvent)
            {
                handleNativeEvent(nativeEvent);
            }
        );
    }
}

GameOfLifeRenderPass::~GameOfLifeRenderPass()
{
    // Detach from the event source so we never forward events to a destroyed
    // pass. Only clear the callback if it is still ours.
    if (eventSource_)
    {
        eventSource_->setEventCallback(nullptr);
        eventSource_ = nullptr;
    }

    if (graphicsProgram_ && pipeline_)
    {
        pipeline_->destroyProgram(graphicsProgram_);
        graphicsProgram_ = 0;
        pipeline_ = nullptr;
    }
}

void GameOfLifeRenderPass::handleNativeEvent(
    const void* nativeEvent)
{
    if (!eventSource_ || !nativeEvent)
        return;

    const auto* event = static_cast<const SDL_Event*>(nativeEvent);

    switch (event->type)
    {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event->button.button == SDL_BUTTON_LEFT)
        {
            mouseDragging_ = true;
            lastMouse_ = {
                event->button.x,
                event->button.y
            };
        }
        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event->button.button == SDL_BUTTON_LEFT)
            mouseDragging_ = false;
        break;

    case SDL_EVENT_MOUSE_MOTION:
        if (mouseDragging_)
        {
            const Vec2 current = {
                event->motion.x,
                event->motion.y
            };

            camera_.pan({
                lastMouse_.x - current.x,
                lastMouse_.y - current.y
            });
            
            lastMouse_ = current;
        }
        break;

    case SDL_EVENT_MOUSE_WHEEL:
    {
        // Scroll up (positive Y, away from user) zooms in.
        const float factor = std::pow(1.1f, event->wheel.y);

        camera_.zoomAt(
            {
                event->wheel.mouse_x,
                event->wheel.mouse_y
            },
            factor,
            static_cast<float>(renderer_.windowWidth()),
            static_cast<float>(renderer_.windowHeight())
        );
        break;
    }

    default:
        break;
    }
}

void GameOfLifeRenderPass::render(
    interfaces::render_pass::RenderPassContext& context)
{
    auto& renderer = context.renderer;

    if (!dataSource_)
        return;

    const auto data = dataSource_();

    // GPU mode renders from the raw GPU grid buffers (no host readback); the
    // CPU/camera path below only applies to the CPU backends' aliveCells list.
    if (data.mode == module::cgol::GameOfLifeDataMode::GPU)
    {
        renderGpu(data, renderer);
        return;
    }

    if (data.width == 0 || data.height == 0 || data.aliveCells.empty())
        return;

    const float screenW =
        static_cast<float>(renderer.windowWidth());

    const float screenH =
        static_cast<float>(renderer.windowHeight());

    // First time we see the data, fit the camera over the live-cell bounding
    // box so the pattern is visible before the user takes over with the mouse.
    if (!cameraInitialized_)
    {
        float minX = static_cast<float>(data.aliveCells[0].x);
        float maxX = minX;
        float minY = static_cast<float>(data.aliveCells[0].y);
        float maxY = minY;

        for (const auto cell : data.aliveCells)
        {
            const float fx = static_cast<float>(cell.x);
            const float fy = static_cast<float>(cell.y);

            minX = std::min(minX, fx);
            maxX = std::max(maxX, fx);
            minY = std::min(minY, fy);
            maxY = std::max(maxY, fy);
        }

        camera_.position = {
            (minX + maxX) * 0.5f,
            (minY + maxY) * 0.5f
        };

        const float fitW = (maxX - minX) + 4.0f;
        const float fitH = (maxY - minY) + 4.0f;

        camera_.zoom = std::clamp(
            std::min(
                screenW / fitW,
                screenH / fitH
            ),
            0.05f,
            1000.0f
        );

        cameraInitialized_ = true;
    }

    // A live cell is a 1x1 (world-unit) rectangle. The camera maps grid/world
    // coordinates to screen pixels, so the on-screen cell size is exactly one
    // world unit times the zoom factor.
    const float cellSize = camera_.zoom;

    // Iterate only the live cells (O(live cells)), exactly as the original
    // per-backend render loops did. We never scan the full grid here. Cells
    // fully outside the view frustum are skipped (cheap manual clip).
    for (const auto cell : data.aliveCells)
    {
        const Vec2 origin = camera_.worldToScreen(
            {
                static_cast<float>(cell.x),
                static_cast<float>(cell.y)
            },
            screenW,
            screenH
        );

        if (origin.x > screenW ||
            origin.y > screenH ||
            origin.x + cellSize < 0.0f ||
            origin.y + cellSize < 0.0f)
        {
            continue;
        }

        if (cellSize < 1.0f)
        {
            // Sub-pixel cell: render a single point at the cell center.
            renderer.drawPoint(
                origin.x + cellSize * 0.5f,
                origin.y + cellSize * 0.5f,
                0xFFFFFFFF
            );
        }
        else
        {
            renderer.drawRect(
                origin.x,
                origin.y,
                cellSize,
                cellSize,
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

void GameOfLifeRenderPass::renderGpu(
    const module::cgol::GameOfLifeData& data,
    interfaces::backend::IRenderer& renderer)
{
    if (data.gridBuffer == 0 || data.renderConfigBuffer == 0)
        return;

    if (!pipeline_)
    {
        auto* pipeline = dynamic_cast<
            interfaces::backend::IRendererShaderPipeline*>(&renderer);

        if (!pipeline)
            return;

        pipeline_ = pipeline;
        graphicsProgram_ = pipeline_->createGraphicsProgram(
            "assets/shaders/cgol_ver.ver",
            "assets/shaders/cgol_frag.frag");

        if (!graphicsProgram_)
            return;
    }

    // Draw one vertex per grid cell directly from the GPU state buffers. The
    // vertex shader hides dead cells (transparent), so we never need the host
    // to read the grid back to know how many live cells to draw.
    pipeline_->bindProgram(graphicsProgram_);
    pipeline_->bindStorageBuffer(0, data.gridBuffer);
    pipeline_->bindStorageBuffer(1, data.renderConfigBuffer);
    pipeline_->drawPoints(static_cast<u32>(data.width * data.height));

    renderer.drawText(
        std::to_string(data.generation) + " generations (GPU)",
        10.0f,
        10.0f,
        15.0f
    );
}

} // namespace emper::sample