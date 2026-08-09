#pragma once

#include "ComputeTypes.h"
#include <emper/Emper_Engine.h>

#include <OpenGLComputeBackend.h>

namespace emper::backend
{

class IComputeBackend;

class Compute
{
public:
    explicit Compute(
        OpenGLComputeBackend& backend
    );

    bool initialize();
    void shutdown();

    void run(emper::u32 particleCount);

    BufferHandle buffer() const
    {
        return buffer_;
    }

private:
    OpenGLComputeBackend& backend_;

    ProgramHandle program_ = 0;
    BufferHandle buffer_ = 0;
};

}