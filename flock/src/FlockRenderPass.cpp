#include "FlockRenderPass.h"

#include <FlockFrameStats.h>

#include <array>
#include <string>

namespace emper::sample
{

namespace
{

// Colors use the 0xRRGGBBAA layout (matches the previous CPU renderer).
constexpr std::array<u32, 3> TeamColors{
    0xFF4D4DFF, 0x4DFF88FF, 0x4D8DFFFF
};

} // namespace

FlockRenderPass::FlockRenderPass(
    module::Flock& flock,
    interfaces::backend::IRenderer& renderer)
    : flock_(flock)
    , renderer_(renderer)
{
}

FlockRenderPass::~FlockRenderPass()
{
    if (graphicsProgram_ && pipeline_)
        pipeline_->destroyProgram(graphicsProgram_);
    graphicsProgram_ = 0;
    pipeline_ = nullptr;
}

void FlockRenderPass::render(
    interfaces::render_pass::RenderPassContext& context)
{
    auto& renderer = context.renderer;

    // Mirror the previous CPU-only render behaviour: keep the simulation's
    // world dimensions in sync with the drawable surface. GPU mode drives
    // its layout from the render config buffer and is left untouched.
    if (flock_.computeMode() == interfaces::module::ComputeMode::CPU)
    {
        const int windowWidth = renderer.windowWidth();
        const int windowHeight = renderer.windowHeight();
        if (windowWidth > 0 && windowHeight > 0)
        {
            flock_.synchronizeWorldSize(
                static_cast<f32>(windowWidth),
                static_cast<f32>(windowHeight));
        }
    }

    const module::FlockData data = flock_.data();

    if (data.mode == module::FlockDataMode::GPU)
        renderGpu(data, renderer);
    else
        renderCpu(data, renderer);
}

void FlockRenderPass::renderCpu(
    const module::FlockData& data,
    interfaces::backend::IRenderer& renderer)
{
    const auto count = data.positions.size();

    for (std::size_t i = 0; i < count; ++i)
    {
        const auto team =
            data.teams[i] % TeamColors.size();

        renderer.drawPoint(
            data.positions[i].x,
            data.positions[i].y,
            TeamColors[team]);
    }
}

void FlockRenderPass::renderGpu(
    const module::FlockData& data,
    interfaces::backend::IRenderer& renderer)
{
    if (data.stateBuffer == 0)
        return;

    if (!pipeline_)
    {
        auto* pipeline = dynamic_cast<
            interfaces::backend::IRendererShaderPipeline*>(&renderer);
        if (!pipeline)
            return;

        pipeline_ = pipeline;
        graphicsProgram_ = pipeline_->createGraphicsProgram(
            "assets/shaders/flock_ver.ver",
            "assets/shaders/flock_frag.frag");
        if (!graphicsProgram_)
            return;
    }

    pipeline_->bindProgram(graphicsProgram_);
    pipeline_->bindStorageBuffer(0, data.stateBuffer);
    pipeline_->bindStorageBuffer(2, data.teamBuffer);
    pipeline_->bindStorageBuffer(5, data.renderConfigBuffer);
    pipeline_->drawPoints(static_cast<u32>(data.boidCount));

    // Benchmark / debug metrics (previously printed inside the compute
    // implementation). The render pass consumes the stats through the
    // public Flock API; they are only produced when benchmark mode is on.
    module::FlockFrameStats stats;
    if (flock_.lastFrameStats(stats))
    {
        renderer.drawText(
            "Candidates: " + std::to_string(stats.candidateChecks),
            20.0f, 60.0f, 10.0f);
        renderer.drawText(
            "Neighbours: " + std::to_string(stats.neighbours),
            20.0f, 72.0f, 10.0f);
        renderer.drawText(
            "Max Candidates: " + std::to_string(stats.maxCandidatesPerBoid),
            20.0f, 84.0f, 10.0f);
        renderer.drawText(
            "Max Neighbours: " + std::to_string(stats.maxNeighboursPerBoid),
            20.0f, 96.0f, 10.0f);
    }
}

} // namespace emper::sample