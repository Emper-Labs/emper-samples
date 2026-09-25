#pragma once

#include <emper/interfaces/backend/IRenderer.h>
#include <emper/interfaces/render-pass/IRenderPass.h>

#include <GameOfLifeData.h>

#include <functional>

namespace emper::sample
{

//test cam before we bring to engine
struct CGoLCamera
{
    Vec2 position{0.0f, 0.0f};
    float zoom = 10.0f;

    Vec2 worldToScreen(
        Vec2 world,
        float screenWidth,
        float screenHeight) const;

    Vec2 screenToWorld(
        Vec2 screen,
        float screenWidth,
        float screenHeight) const;

    void pan(Vec2 delta);
    void zoomAt(
        Vec2 screenPosition,
        float factor,
        float screenWidth,
        float screenHeight);
};

// Visualization pass for the Game of Life simulation. This lives in the
// application/rendering layer (NOT inside the simulation-only
// emper-module-CGoL). It consumes the renderer-neutral GameOfLifeData produced
// by the simulation via data() and drives an IRenderer.
//
// Dependency chain:
//   GameOfLife --data()--> GameOfLifeData -> GameOfLifeRenderPass
//     -> RenderPassContext / IRenderer
//
// The pass never mutates simulation state and takes no ownership of the
// simulation. It is backend-agnostic: the data source is a small callable
// produced from whichever GameOfLife backend the application instantiated.
class GameOfLifeRenderPass final
    : public interfaces::render_pass::IRenderPass
{
public:
    using DataSource =
        std::function<module::cgol::GameOfLifeData()>;

    explicit GameOfLifeRenderPass(
        DataSource dataSource,
        interfaces::backend::IRenderer& renderer);

    ~GameOfLifeRenderPass() override;

    GameOfLifeRenderPass(const GameOfLifeRenderPass&) = delete;
    GameOfLifeRenderPass& operator=(const GameOfLifeRenderPass&) = delete;

    void render(
        interfaces::render_pass::RenderPassContext& context
    ) override;

private:
    // Consumes native events forwarded by the renderer. The payload is opaque
    // here (const void*); the implementation casts it to a backend event (SDL)
    // so the header stays backend-agnostic.
    void handleNativeEvent(const void* nativeEvent);

    DataSource dataSource_;
    CGoLCamera camera_;

    interfaces::backend::IRenderer& renderer_;

    interfaces::behavior::INativeEventSource* eventSource_ = nullptr;

    bool mouseDragging_ = false;
    Vec2 lastMouse_{0.0f, 0.0f};

    bool cameraInitialized_ = false;
};

} // namespace emper::sample