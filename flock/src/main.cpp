#include <emper/Emper_Engine.h>

#include <emper/interfaces/backend/IRenderer.h>
#include <emper/interfaces/render-pass/IRenderPass.h>

#include <OpenGLComputeBackend.h>
#include <SDLOpenGLRenderer.h>
#include <Flock.h>

#include "FlockRenderPass.h"

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>

#include <SDL3/SDL.h>

#include <filesystem>
#include <iostream>


class MyImGuiPass final
    : public emper::interfaces::render_pass::IRenderPass
{
public:
    explicit MyImGuiPass(
        emper::interfaces::backend::IRenderer& renderer)
        : renderer_(renderer)
    {
        initialize();
    }

    ~MyImGuiPass() override
    {
        shutdown();
    }

    void render(
        emper::interfaces::render_pass::RenderPassContext& context
    ) override
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Emper");

        ImGui::Text("Emper Engine");

        ImGui::Separator();

        ImGui::Text(
            "FPS: %.1f",
            context.fps
        );

        ImGui::Text(
            "Delta time: %.3f ms",
            context.deltaTime * 1000.0f
        );

        ImGui::End();

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(
            ImGui::GetDrawData()
        );
    }

private:
    void initialize()
    {
        auto* sdlRenderer =
            dynamic_cast<
                emper::backend::SDLOpenGLRenderer*
            >(&renderer_);

        if (!sdlRenderer)
        {
            std::cerr
                << "MyImGuiPass: renderer is not "
                   "SDLOpenGLRenderer\n";

            return;
        }

        IMGUI_CHECKVERSION();

        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        if (!ImGui_ImplSDL3_InitForOpenGL(
                sdlRenderer->nativeWindow(),
                sdlRenderer->nativeGLContext()))
        {
            std::cerr
                << "MyImGuiPass: "
                   "ImGui_ImplSDL3_InitForOpenGL failed\n";

            ImGui::DestroyContext();
            return;
        }

        if (!ImGui_ImplOpenGL3_Init("#version 330"))
        {
            std::cerr
                << "MyImGuiPass: "
                   "ImGui_ImplOpenGL3_Init failed\n";

            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();
            return;
        }

        auto* events =
            dynamic_cast<
                emper::interfaces::behavior::INativeEventSource*
            >(&renderer_);

        if (events)
        {
            events->setEventCallback(
                [](const void* nativeEvent)
                {
                    if (!nativeEvent)
                        return;

                    ImGui_ImplSDL3_ProcessEvent(
                        static_cast<const SDL_Event*>(
                            nativeEvent
                        )
                    );
                }
            );

            eventSource_ = events;
        }
        else
        {
            std::cerr
                << "MyImGuiPass: renderer does not provide "
                   "INativeEventSource\n";
        }
    }

    void shutdown()
    {
        if (eventSource_)
        {
            eventSource_->setEventCallback(nullptr);
            eventSource_ = nullptr;
        }


        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();

        ImGui::DestroyContext();
    }

private:
    emper::interfaces::backend::IRenderer& renderer_;

    emper::interfaces::behavior::INativeEventSource*
        eventSource_ = nullptr;
};


int main(int argc, char** argv)
{

    if (argc > 0 && argv[0])
    {
        std::filesystem::current_path(
            std::filesystem::absolute(argv[0]).parent_path()
        );
    }

    emper::simulation::Simulation simulation;

    simulation.initialize();


    emper::backend::OpenGLComputeBackend computeBackend;

    emper::backend::SDLOpenGLRenderer renderer(
        "Emper Flock",
        1280,
        720
    );


    simulation.setRenderer(renderer);


    auto& world = simulation.world();

    emper::module::FlockConfig config;

    /*
     * Mode
     */
    config.mode =
        emper::interfaces::module::ComputeMode::GPU;

    config.boidCount = 100000;

    config.worldWidth  = 1280.0f;
    config.worldHeight = 720.0f;

    config.maxSpeed     = 80.0f;
    config.initialSpeed = 40.0f;
    config.maxForce     = 25.0f;

    config.perceptionRadius = 50.0f;
    config.separationRadius = 30.0f;

    config.separationWeight = 4.0f;
    config.alignmentWeight  = 1.0f;
    config.cohesionWeight   = 1.0f;

    config.maxNeighbours = 64;
    config.teamCount = 3;

    emper::module::Flock flock(
        world,
        config,
        &computeBackend
    );

    simulation.addSystem(flock);


    emper::sample::FlockRenderPass flockRenderPass(
        flock,
        renderer
    );

    simulation.addRenderPass(flockRenderPass);


    MyImGuiPass imguiPass(renderer);

    simulation.addRenderPass(imguiPass);


    simulation.start();

    while (simulation.isRunning())
    {
        simulation.tick();
    }
    simulation.shutdown();

    return 0;
}