#pragma once

#include <emper/interfaces/backend/IRenderer.h>
#include <emper/interfaces/render-pass/IRenderPass.h>

#include <Flock.h>
#include <FlockData.h>

namespace emper::sample
{

// Visualization pass for the Flock simulation. This lives in the
// application/rendering layer (NOT inside the simulation-only
// emper-module-flock). It consumes the general-purpose FlockData produced
// by the Flock simulation and drives a renderer / shader pipeline.
//
// Dependency chain:
//   FlockData -> FlockRenderPass -> IRenderer / IRendererShaderPipeline
class FlockRenderPass final
    : public interfaces::render_pass::IRenderPass
{
public:
    // The flock must outlive this pass (non-owning reference).
    FlockRenderPass(
        module::Flock& flock,
        interfaces::backend::IRenderer& renderer);
    ~FlockRenderPass() override;

    FlockRenderPass(const FlockRenderPass&) = delete;
    FlockRenderPass& operator=(const FlockRenderPass&) = delete;

    void render(
        interfaces::render_pass::RenderPassContext& context
    ) override;

private:
    void renderCpu(
        const module::FlockData& data,
        interfaces::backend::IRenderer& renderer);
    void renderGpu(
        const module::FlockData& data,
        interfaces::backend::IRenderer& renderer);

    module::Flock& flock_;
    interfaces::backend::IRenderer& renderer_;

    // GPU path: lazily created graphics program bound to the FlockData GPU
    // buffers. Only used when the simulation is in GPU mode.
    interfaces::backend::IRendererShaderPipeline* pipeline_ = nullptr;
    emper::ProgramHandle graphicsProgram_ = 0;
};

} // namespace emper::sample