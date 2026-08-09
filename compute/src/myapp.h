#pragma once

#include "compute.h"
#include "OpenGLComputeBackend.h"
#include "SDLOpenGLRenderer.h"

class MyApp
{
public:
    MyApp();
    ~MyApp() = default;

    bool initialize();
    void run();
    void shutdown();

private:
    void render();

private:
    emper::backend::OpenGLComputeBackend backend_;
    emper::backend::SDLOpenGLRenderer renderer_;
    emper::backend::Compute compute_;
    emper::ProgramHandle graphicsProgram_ = 0;

};