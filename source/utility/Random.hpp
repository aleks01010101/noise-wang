#pragma once

#include "Types.hpp"

struct Random
{
    Random() = delete;
    Random(const Random&) = default;
    Random(Random&&) = default;
    explicit Random(u32 seed);
    ~Random() = default;
    
    u32 Next();
    
    // Uniform distribution
    f32 Uniform();
    f32 Uniform(f32 range);
    f32 Uniform(f32 start, f32 end);
    u32 Uniform(u32 start, u32 end);

    // Poisson distribution
    u32 Poisson(f32 mean);

private:
    u32 x;
};
