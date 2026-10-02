#include "ParticleRenderPass.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <string>

namespace emper::sample
{

namespace
{
constexpr f32 kPi = 3.14159265358979323846f;

// Reference grid / axes extent and spacing (world units).
constexpr f32 kGridHalf = 6.0f;
constexpr f32 kGridStep = 1.0f;
constexpr int kGridLines = 2 * static_cast<int>(kGridHalf / kGridStep) + 1;
}

float ParticleCamera::focal(float screenHeight) const
{
    return (screenHeight * 0.5f) /
        std::tan((fov * 0.5f) * (kPi / 180.0f));
}

Vec3 ParticleCamera::right() const
{
    return { std::cos(yaw), 0.0f, std::sin(yaw) };
}

Vec3 ParticleCamera::up() const
{
    return {
        -std::sin(pitch) * std::sin(yaw),
         std::cos(pitch),
         std::sin(pitch) * std::cos(yaw)
    };
}

bool ParticleCamera::project(
    Vec3 world,
    float screenW,
    float screenH,
    Vec2& out) const
{
    const Vec3 t = {
        world.x - position.x,
        world.y - position.y,
        world.z - position.z
    };

    // Transform the world point into camera space by rotating -yaw around Y
    // then -pitch around X. The camera looks down its -Z axis.
    const f32 cy = std::cos(-yaw);
    const f32 sy = std::sin(-yaw);
    const f32 x1 = t.x * cy + t.z * sy;
    const f32 y1 = t.y;
    const f32 z1 = -t.x * sy + t.z * cy;

    const f32 cp = std::cos(-pitch);
    const f32 sp = std::sin(-pitch);
    const f32 x2 = x1;
    const f32 y2 = y1 * cp - z1 * sp;
    const f32 z2 = y1 * sp + z1 * cp;

    if (z2 <= 0.001f)
        return false;

    const f32 f = focal(screenH);
    out = {
        screenW * 0.5f + (x2 * f) / z2,
        screenH * 0.5f - (y2 * f) / z2
    };
    return true;
}

void ParticleCamera::orbit(f32 dyaw, f32 dpitch)
{
    yaw += dyaw;
    pitch += dpitch;

    const f32 maxPitch = 89.0f * (kPi / 180.0f);
    pitch = std::clamp(pitch, -maxPitch, maxPitch);
}

void ParticleCamera::pan(f32 dxScreen, f32 dyScreen)
{
    const f32 scale = 0.003f;
    const Vec3 r = right();
    const Vec3 u = up();

    position.x += r.x * dxScreen * scale;
    position.y += r.y * dxScreen * scale;
    position.z += r.z * dxScreen * scale;

    position.x -= u.x * dyScreen * scale;
    position.y -= u.y * dyScreen * scale;
    position.z -= u.z * dyScreen * scale;
}

void ParticleCamera::zoom(f32 factor)
{
    fov = std::clamp(fov / factor, 2.0f, 160.0f);
}

ParticleRenderPass::ParticleRenderPass(
    modules::newton_mechanics::NewtonSystem& system,
    interfaces::backend::IRenderer& renderer)
    : system_(system),
      renderer_(renderer)
{
    auto* eventSource =
        dynamic_cast<interfaces::behavior::INativeEventSource*>(&renderer_);

    if (eventSource)
    {
        eventSource_ = eventSource;
        eventSource_->setEventCallback(
            [this](const void* nativeEvent)
            {
                handleNativeEvent(nativeEvent);
            }
        );
    }
}

ParticleRenderPass::~ParticleRenderPass()
{
    if (eventSource_)
    {
        eventSource_->setEventCallback(nullptr);
        eventSource_ = nullptr;
    }
}

void ParticleRenderPass::handleNativeEvent(
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
            leftDragging_ = true;
            lastMouse_ = {
                static_cast<float>(event->button.x),
                static_cast<float>(event->button.y)
            };
        }
        else if (event->button.button == SDL_BUTTON_RIGHT)
        {
            rightDragging_ = true;
            lastMouse_ = {
                static_cast<float>(event->button.x),
                static_cast<float>(event->button.y)
            };
        }
        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event->button.button == SDL_BUTTON_LEFT)
            leftDragging_ = false;
        else if (event->button.button == SDL_BUTTON_RIGHT)
            rightDragging_ = false;
        break;

    case SDL_EVENT_MOUSE_MOTION:
    {
        const Vec2 current = {
            static_cast<float>(event->motion.x),
            static_cast<float>(event->motion.y)
        };

        if (leftDragging_)
        {
            camera_.orbit(
                (current.x - lastMouse_.x) * 0.01f,
                (current.y - lastMouse_.y) * 0.01f
            );
        }
        else if (rightDragging_)
        {
            camera_.pan(
                current.x - lastMouse_.x,
                current.y - lastMouse_.y
            );
        }

        lastMouse_ = current;
        break;
    }

    case SDL_EVENT_MOUSE_WHEEL:
        camera_.zoom(std::pow(1.1f, event->wheel.y));
        break;

    default:
        break;
    }
}

void ParticleRenderPass::drawWorld(
    interfaces::backend::IRenderer& renderer,
    f32 screenW,
    f32 screenH)
{
    // Reference grid on the y=0 plane (world XZ plane).
    const u32 gridColor = 0x606060FF;

    for (int i = 0; i < kGridLines; ++i)
    {
        const f32 v = -kGridHalf + i * kGridStep;

        Vec2 a, b;
        const bool va = camera_.project({ v, 0.0f, -kGridHalf }, screenW, screenH, a);
        const bool vb = camera_.project({ v, 0.0f,  kGridHalf }, screenW, screenH, b);
        if (va && vb)
        {
            renderer.drawLine(
                a.x, screenH - a.y,
                b.x, screenH - b.y,
                gridColor
            );
        }

        Vec2 c, d;
        const bool vc = camera_.project({ -kGridHalf, 0.0f, v }, screenW, screenH, c);
        const bool vd = camera_.project({  kGridHalf, 0.0f, v }, screenW, screenH, d);
        if (vc && vd)
        {
            renderer.drawLine(
                c.x, screenH - c.y,
                d.x, screenH - d.y,
                gridColor
            );
        }
    }

    // World coordinate axes: X red, Y green, Z blue.
    const Vec3 axes[3] = {
        { kGridHalf, 0.0f, 0.0f },
        { 0.0f, kGridHalf, 0.0f },
        { 0.0f, 0.0f, kGridHalf }
    };
    const u32 axisColors[3] = {
        0xFF0000FF, // X red
        0x00FF00FF, // Y green
        0x0000FFFF  // Z blue
    };
    const std::string axisLabels[3] = { "X", "Y", "Z" };

    Vec2 o;
    const bool oVisible = camera_.project({ 0.0f, 0.0f, 0.0f }, screenW, screenH, o);

    for (int i = 0; i < 3; ++i)
    {
        Vec2 t;
        if (oVisible && camera_.project(axes[i], screenW, screenH, t))
        {
            renderer.drawLine(
                o.x, screenH - o.y,
                t.x, screenH - t.y,
                axisColors[i]
            );
            renderer.drawText(axisLabels[i], t.x, t.y, 14.0f, axisColors[i]);
        }
    }
}

void ParticleRenderPass::render(
    interfaces::render_pass::RenderPassContext& context)
{
    auto& renderer = context.renderer;
    const auto& particles = system_.particles();

    const f32 screenW = static_cast<f32>(renderer.windowWidth());
    const f32 screenH = static_cast<f32>(renderer.windowHeight());
    if (screenW <= 0.0f || screenH <= 0.0f)
        return;

    drawWorld(renderer, screenW, screenH);

    const u32 bodyColor = 0xFF8FBFFF;
    const u32 tailColor = 0xFFCCD0FF;

    for (std::size_t i = 0; i < particles.size(); ++i)
    {
        const auto& p = particles[i];

        Vec2 body;
        if (!camera_.project(p.position, screenW, screenH, body))
            continue;

        // World-space body radius, then convert to on-screen pixels by
        // projecting a probe point offset along the camera-right axis.
        const f32 worldRadius = std::clamp(p.mass * 0.15f, 0.02f, 5.0f);
        const Vec3 r = camera_.right();

        Vec2 probe;
        camera_.project(
            {
                p.position.x + r.x * worldRadius,
                p.position.y + r.y * worldRadius,
                p.position.z + r.z * worldRadius
            },
            screenW,
            screenH,
            probe);

        const f32 radius = std::clamp(
            std::hypot(probe.x - body.x, probe.y - body.y),
            1.0f,
            60.0f
        );

        renderer.drawCircle(body.x, body.y, radius, bodyColor);

        // 3D velocity tail.
        Vec2 tip;
        if (camera_.project(
                {
                    p.position.x + p.velocity.x * 0.5f,
                    p.position.y + p.velocity.y * 0.5f,
                    p.position.z + p.velocity.z * 0.5f
                },
                screenW,
                screenH,
                tip))
        {
            renderer.drawLine(
                body.x, screenH - body.y,
                tip.x, screenH - tip.y,
                tailColor
            );
        }
    }
}

} // namespace emper::sample