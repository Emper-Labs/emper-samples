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

ParticleCamera::ParticleCamera()
{
    // Initial camera:
    //
    // target = (0, 0, 0)
    // distance = 10
    // yaw = 0
    // pitch = 0
    //
    // Camera therefore starts at:
    //
    // (0, 0, 10)

    updatePosition();
}

float ParticleCamera::focal(float screenHeight) const
{
    return (screenHeight * 0.5f) /
        std::tan((fov * 0.5f) * (kPi / 180.0f));
}

Vec3 ParticleCamera::forward() const
{
    const f32 cp = std::cos(pitch);

    return {
        -cp * std::sin(yaw),
        std::sin(pitch),
        -cp * std::cos(yaw)
    };
}

Vec3 ParticleCamera::right() const
{
    return {
        std::cos(yaw),
        0.0f,
        -std::sin(yaw)
    };
}

Vec3 ParticleCamera::up() const
{
    const Vec3 r = right();
    const Vec3 f = forward();

    return {
        r.y * f.z - r.z * f.y,
        r.z * f.x - r.x * f.z,
        r.x * f.y - r.y * f.x
    };
}

void ParticleCamera::updatePosition()
{
    const Vec3 f = forward();

    position = {
        target.x - f.x * distance,
        target.y - f.y * distance,
        target.z - f.z * distance
    };
}

void ParticleCamera::orbit(f32 dyaw, f32 dpitch)
{
    yaw += dyaw;
    pitch += dpitch;

    const f32 maxPitch = 89.0f * (kPi / 180.0f);

    pitch = std::clamp(
        pitch,
        -maxPitch,
        maxPitch
    );

    updatePosition();
}

void ParticleCamera::pan(
    f32 dxScreen,
    f32 dyScreen,
    f32 screenHeight)
{
    if (screenHeight <= 0.0f)
        return;

    /*
     * Perspective projection:
     *
     *     screen = world * focal / depth
     *
     * Therefore approximately:
     *
     *     worldPerPixel = distance / focal
     *
     * This makes pan speed depend on camera distance.
     */

    const f32 f = focal(screenHeight);

    if (f <= 0.0f)
        return;

    const f32 worldPerPixel = distance / f;

    const Vec3 r = right();
    const Vec3 u = up();

    const Vec3 delta = {
        r.x * dxScreen * worldPerPixel +
            u.x * dyScreen * worldPerPixel,

        r.y * dxScreen * worldPerPixel +
            u.y * dyScreen * worldPerPixel,

        r.z * dxScreen * worldPerPixel +
            u.z * dyScreen * worldPerPixel
    };

    position.x += delta.x;
    position.y += delta.y;
    position.z += delta.z;

    target.x += delta.x;
    target.y += delta.y;
    target.z += delta.z;
}

void ParticleCamera::zoom(f32 factor)
{
    if (factor <= 0.0f)
        return;

    /*
     * factor > 1:
     *     zoom in
     *
     * factor < 1:
     *     zoom out
     */

    distance /= factor;

    distance = std::clamp(
        distance,
        kMinDistance,
        kMaxDistance
    );

    updatePosition();
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

    /*
     * Camera basis:
     *
     * right
     * up
     * forward
     *
     * Camera looks along -Z in camera space.
     */

    const Vec3 r = right();
    const Vec3 u = up();
    const Vec3 f = forward();

    const f32 x2 =
        t.x * r.x +
        t.y * r.y +
        t.z * r.z;

    const f32 y2 =
        t.x * u.x +
        t.y * u.y +
        t.z * u.z;

    const f32 z2 =
        (t.x * f.x +
          t.y * f.y +
          t.z * f.z);

    if (z2 <= 0.001f)
        return false;

    const f32 focalLength = focal(screenH);

    out = {
        screenW * 0.5f +
            (x2 * focalLength) / z2,

        screenH * 0.5f -
            (y2 * focalLength) / z2
    };

    return true;
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
                current.y - lastMouse_.y,
                static_cast<f32>(renderer_.windowHeight())
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

ParticleRenderPass::ParticleRenderPass(
    modules::classical_mechanics::ClassicalMechanicsSystem& system,
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

    // XZ plane (Y = 0)
    {
        Vec2 a, b;

        const bool va = camera_.project(
            { v, 0.0f, -kGridHalf },
            screenW,
            screenH,
            a
        );

        const bool vb = camera_.project(
            { v, 0.0f, kGridHalf },
            screenW,
            screenH,
            b
        );

        if (va && vb)
        {
            renderer.drawLine(
                a.x, screenH - a.y,
                b.x, screenH - b.y,
                gridColor
            );
        }
    }

    {
        Vec2 a, b;

        const bool va = camera_.project(
            { -kGridHalf, 0.0f, v },
            screenW,
            screenH,
            a
        );

        const bool vb = camera_.project(
            { kGridHalf, 0.0f, v },
            screenW,
            screenH,
            b
        );

        if (va && vb)
        {
            renderer.drawLine(
                a.x, screenH - a.y,
                b.x, screenH - b.y,
                gridColor
            );
        }
    }

    // XY plane (Z = 0)
    {
        Vec2 a, b;

        const bool va = camera_.project(
            { v, -kGridHalf, 0.0f },
            screenW,
            screenH,
            a
        );

        const bool vb = camera_.project(
            { v, kGridHalf, 0.0f },
            screenW,
            screenH,
            b
        );

        if (va && vb)
        {
            renderer.drawLine(
                a.x, screenH - a.y,
                b.x, screenH - b.y,
                gridColor
            );
        }
    }

    {
        Vec2 a, b;

        const bool va = camera_.project(
            { -kGridHalf, v, 0.0f },
            screenW,
            screenH,
            a
        );

        const bool vb = camera_.project(
            { kGridHalf, v, 0.0f },
            screenW,
            screenH,
            b
        );

        if (va && vb)
        {
            renderer.drawLine(
                a.x, screenH - a.y,
                b.x, screenH - b.y,
                gridColor
            );
        }
    }

    // YZ plane (X = 0)
    {
        Vec2 a, b;

        const bool va = camera_.project(
            { 0.0f, v, -kGridHalf },
            screenW,
            screenH,
            a
        );

        const bool vb = camera_.project(
            { 0.0f, v, kGridHalf },
            screenW,
            screenH,
            b
        );

        if (va && vb)
        {
            renderer.drawLine(
                a.x, screenH - a.y,
                b.x, screenH - b.y,
                gridColor
            );
        }
    }

    {
        Vec2 a, b;

        const bool va = camera_.project(
            { 0.0f, -kGridHalf, v },
            screenW,
            screenH,
            a
        );

        const bool vb = camera_.project(
            { 0.0f, kGridHalf, v },
            screenW,
            screenH,
            b
        );

        if (va && vb)
        {
            renderer.drawLine(
                a.x, screenH - a.y,
                b.x, screenH - b.y,
                gridColor
            );
        }
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

void ParticleRenderPass::drawVectorArrow(
    interfaces::backend::IRenderer& renderer,
    f32 screenW,
    f32 screenH,
    const Vec3& worldPos,
    const Vec2& body,
    f32 padRadius,
    const Vec3& vector,
    u32 color)
{
    const f32 mag = std::hypot(vector.x, vector.y, vector.z);
    if (mag <= 0.001f)
        return;

    // World-scaled arrow length (length in world units per velocity unit).
    const f32 kScale = 2.5f;
    const f32 lenWorld = std::clamp(mag * kScale, 0.0f, 4.0f);

    Vec2 tip;
    if (!camera_.project(
            {
                worldPos.x + vector.x / mag * lenWorld,
                worldPos.y + vector.y / mag * lenWorld,
                worldPos.z + vector.z / mag * lenWorld
            },
            screenW,
            screenH,
            tip))
    {
        return;
    }

    f32 dx = tip.x - body.x;
    f32 dy = tip.y - body.y;
    f32 lenS = std::hypot(dx, dy);
    if (lenS <= 1.0f)
        return;

    // Guarantee the arrow is always clearly visible on screen.
    const f32 minLen = 26.0f;
    if (lenS < minLen)
    {
        const f32 k = minLen / lenS;
        dx *= k;
        dy *= k;
        lenS = minLen;
        tip.x = body.x + dx;
        tip.y = body.y + dy;
    }

    const f32 ux = dx / lenS;
    const f32 uy = dy / lenS;
    const f32 px = -uy; // screen-perpendicular
    const f32 py = ux;

    // Shaft starts at the particle's circle edge.
    const f32 startPad = std::clamp(padRadius, 2.0f, lenS * 0.45f);
    const f32 sx = body.x + ux * startPad;
    const f32 sy = body.y + uy * startPad;

    const f32 headLen = std::clamp(lenS * 0.3f, 6.0f, 22.0f);
    const f32 bx = sx + ux * (lenS - startPad - headLen);
    const f32 by = sy + uy * (lenS - startPad - headLen);
    const f32 headHalf = headLen * 0.45f;

    renderer.drawLine(sx, screenH - sy, bx, screenH - by, color);
    renderer.drawLine(
        bx + px * headHalf, screenH - (by + py * headHalf),
        tip.x, screenH - tip.y, color);
    renderer.drawLine(
        bx - px * headHalf, screenH - (by - py * headHalf),
        tip.x, screenH - tip.y, color);
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
    const u32 arrowColor = 0xFFA000FF; // orange: velocity
    const u32 accelColor = 0xFFFF00FF; // magenta: acceleration

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

        // Velocity vector (orange).
        drawVectorArrow(
            renderer, screenW, screenH,
            p.position, body, radius,
            p.velocity, arrowColor
        );

        // Acceleration vector (magenta).
        drawVectorArrow(
            renderer, screenW, screenH,
            p.position, body, radius,
            p.acceleration, accelColor
        );
    }

    renderer.drawText(
        "orange: velocity | magenta: acceleration",
        10.0f,
        30.0f,
        12.0f,
        0xFFFFFFFF
    );

    renderer.drawText(
        "left-drag orbit | right-drag pan | wheel zoom",
        10.0f,
        48.0f,
        12.0f,
        0xFFB0B0FF
    );
}

} // namespace emper::sample