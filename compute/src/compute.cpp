#include "compute.h"

#include <emper/interfaces/backend/ICompute.h>

#include <array>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>
#include <emper/Emper_Engine.h>

namespace emper::backend
{

Compute::Compute(
    OpenGLComputeBackend& backend)
    : backend_(backend)
{
}

bool Compute::initialize()
{
    program_ =
        backend_.compileShader(
            "assets/shaders/particles_compute.comp");

    if (!program_)
        return false;

    constexpr std::size_t ParticleCount = 10'000'000;

    buffer_ =
        backend_.createBuffer({
            sizeof(float) * ParticleCount * 4
        });

    if (!buffer_)
        return false;

    std::vector<float> particles(
        ParticleCount * 4
    );

    std::mt19937 rng(
        std::random_device{}()
    );

    std::uniform_real_distribution<float> posDist(
        -0.9f,
        0.9f
    );

    std::uniform_real_distribution<float> velDist(
        -0.4f,
        0.4f
    );

    for (std::size_t i = 0;
        i < ParticleCount;
        ++i)
    {
        particles[i * 4 + 0] = posDist(rng);
        particles[i * 4 + 1] = posDist(rng);
        particles[i * 4 + 2] = velDist(rng);
        particles[i * 4 + 3] = velDist(rng);
    }

    backend_.writeBuffer(
        buffer_,
        particles.data(),
        particles.size() * sizeof(float)
    );

    backend_.bindStorageBuffer(
        0,
        buffer_
    );

    return true;
}

void Compute::run(emper::u32 particleCount)
{
    constexpr u32 LocalSize = 256;

    backend_.setUniform1f(
        program_,
        "dt",
        0.016f
    );

    backend_.setUniform1i(
        program_,
        "particleCount",
        static_cast<int>(particleCount)
    );

    const u32 groups =
        (particleCount + LocalSize - 1) / LocalSize;

    backend_.dispatch(
        program_,
        {
            groups,
            1,
            1
        }
    );

    backend_.memoryBarrier();
}

void Compute::shutdown()
{
    if (buffer_)
    {
        backend_.destroyBuffer(buffer_);
        buffer_ = 0;
    }

    if (program_)
    {
        backend_.destroyProgram(program_);
        program_ = 0;
    }
}

}