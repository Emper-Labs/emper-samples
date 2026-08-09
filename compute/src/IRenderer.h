#pragma once

#include <string>

#include <emper/Emper_Engine.h>

#include "ComputeTypes.h"

using namespace emper;
using emper::compute::ProgramHandle;
using emper::compute::BufferHandle;

class IRenderer //<- cái này chỉ test sau sẽ thêm vào cho engine
{
public:
    virtual ~IRenderer() = default;

    virtual ProgramHandle createGraphicsProgram(
        const std::string& vertex,
        const std::string& fragment
    ) = 0;

    virtual void destroyProgram(
        ProgramHandle program
    ) = 0;

    virtual void bindProgram(
        ProgramHandle program
    ) = 0;

    virtual void bindStorageBuffer(
        u32 binding,
        BufferHandle buffer
    ) = 0;

    virtual void drawPoints(
        u32 count
    ) = 0;
};