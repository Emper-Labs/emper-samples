#pragma once

#include <emper/interfaces/backend/IRenderer.h>
#include <emper/interfaces/render-pass/IRenderPass.h>

#include <emper/modules/classical-mechanics/ClassicalMechanicsSystem.h>

namespace emper::sample
{

// Perspective 3D camera. Projects a world Vec3 onto the 2D screen via a
// yaw/pitch orbit and a field-of-view based perspective divide.
class ParticleCamera
{
public:
    ParticleCamera();

    float focal(float screenHeight) const;

    Vec3 right() const;
    Vec3 up() const;
    Vec3 forward() const;

    bool project(
        Vec3 world,
        float screenW,
        float screenH,
        Vec2& out) const;

    void orbit(f32 dyaw, f32 dpitch);

    void pan(
        f32 dxScreen,
        f32 dyScreen,
        f32 screenHeight);

    void zoom(f32 factor);

private:
    void updatePosition();

private:
    Vec3 position { 0.0f, 5.0f, 10.0f };
    Vec3 target   { 0.0f, 0.0f, 0.0f };

    f32 yaw   = 0.0f;
    f32 pitch = 0.0f;

    f32 distance = 10.0f;

    f32 fov = 60.0f;

    static constexpr f32 kMinDistance = 0.1f;
    static constexpr f32 kMaxDistance = 10000.0f;
    static constexpr f32 kMinFov = 2.0f;
    static constexpr f32 kMaxFov = 160.0f;

    static constexpr f32 kPi = 3.14159265358979323846f;
};

class ParticleRenderPass final
    : public interfaces::render_pass::IRenderPass
{
public:
    ParticleRenderPass(
        modules::classical_mechanics::ClassicalMechanicsSystem& system,
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

    modules::classical_mechanics::ClassicalMechanicsSystem& system_;
    interfaces::backend::IRenderer& renderer_;

    ParticleCamera camera_{};
    interfaces::behavior::INativeEventSource* eventSource_ = nullptr;

    bool leftDragging_ = false;
    bool rightDragging_ = false;
    Vec2 lastMouse_{0.0f, 0.0f};
};

} // namespace emper::sample