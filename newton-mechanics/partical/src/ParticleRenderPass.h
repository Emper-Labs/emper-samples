#pragma once

#include <emper/interfaces/backend/IRenderer.h>
#include <emper/interfaces/render-pass/IRenderPass.h>

#include <emper/modules/newton_mechanics/NewtonSystem.h>

namespace emper::sample
{

// Perspective 3D camera. Projects a world Vec3 onto the 2D screen via a
// yaw/pitch orbit and a field-of-view based perspective divide.
struct ParticleCamera
{
    Vec3 position{0.0f, 0.0f, -8.0f};
    float yaw = 0.0f;
    float pitch = 0.0f;
    float fov = 45.0f; // degrees

    float focal(float screenHeight) const;
    Vec3 right() const;
    Vec3 up() const;

    // Returns false when the point is behind the camera (out stays untouched).
    bool project(Vec3 world, float screenW, float screenH, Vec2& out) const;

    void orbit(float dyaw, float dpitch);
    void pan(float dxScreen, float dyScreen);
    void zoom(float factor);
};

class ParticleRenderPass final
    : public interfaces::render_pass::IRenderPass
{
public:
    ParticleRenderPass(
        modules::newton_mechanics::NewtonSystem& system,
        interfaces::backend::IRenderer& renderer);

    ~ParticleRenderPass() override;

    ParticleRenderPass(const ParticleRenderPass&) = delete;
    ParticleRenderPass& operator=(const ParticleRenderPass&) = delete;

    void render(
        interfaces::render_pass::RenderPassContext& context
    ) override;

private:
    void handleNativeEvent(const void* nativeEvent);
    void drawWorld(
        interfaces::backend::IRenderer& renderer,
        f32 screenW,
        f32 screenH);

    void drawVectorArrow(
        interfaces::backend::IRenderer& renderer,
        f32 screenW,
        f32 screenH,
        const Vec3& worldPos,
        const Vec2& body,
        f32 padRadius,
        const Vec3& vector,
        u32 color);

    modules::newton_mechanics::NewtonSystem& system_;
    interfaces::backend::IRenderer& renderer_;

    ParticleCamera camera_{};
    interfaces::behavior::INativeEventSource* eventSource_ = nullptr;

    bool leftDragging_ = false;
    bool rightDragging_ = false;
    Vec2 lastMouse_{0.0f, 0.0f};
};

} // namespace emper::sample