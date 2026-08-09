#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <glad/gl.h>

namespace emper::compute
{

using BufferHandle = std::uint32_t;
using ProgramHandle = std::uint32_t;

struct BufferDesc
{
    std::size_t size = 0;
};

struct DispatchSize
{
    std::uint32_t x = 1;
    std::uint32_t y = 1;
    std::uint32_t z = 1;
};

struct ProgramBinary
{
    std::string hash;
    GLenum format;
    std::vector<std::byte> data;
};

struct ProgramBinaryFile
{
    char magic[4] = {'E','M','P','R'};
    std::uint32_t version = 1;
    std::string hash;
    GLenum format;
    std::vector<std::byte> data;
};

}