#pragma once

#include <emper/interfaces/backend/IRenderer.h>
#include <emper/interfaces/render-pass/IRenderPass.h>

#include <GameOfLifeData.h>

#include <functional>

namespace emper::sample
{

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
        DataSource dataSource);

    ~GameOfLifeRenderPass() override;

    GameOfLifeRenderPass(const GameOfLifeRenderPass&) = delete;
    GameOfLifeRenderPass& operator=(const GameOfLifeRenderPass&) = delete;

    void render(
        interfaces::render_pass::RenderPassContext& context
    ) override;

private:
    DataSource dataSource_;
};

} // namespace emper::sample