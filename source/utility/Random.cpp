#include "Random.hpp"

#include <cmath>

Random::Random(u32 seed) :
    x(seed > 0 ? seed : 1)
{
}

u32 Random::Next()
{
    x *= 3039177861;
    return x;
}

f32 Random::Uniform()
{
    return static_cast<f32>(static_cast<f64>(Next()) / static_cast<f64>(~0u));
}

f32 Random::Uniform(f32 range)
{
    return range * Uniform();
}

f32 Random::Uniform(f32 start, f32 end)
{
    f32 value = Uniform();
    // linear interpolation in [a; b]:
    // a * (1 - w) + b * w
    // a + w * (b - a)
    // w * length + a
    return fmaf(value, end - start, start);
}

u32 Random::Uniform(u32 start, u32 end)
{
    f32 s = static_cast<f32>(start);
    f32 e = static_cast<f32>(end);
    return static_cast<u32>(floorf(Uniform(s, e) + 0.5f));
}

u32 Random::Poisson(f32 mean)
{
    f32 g = expf(-mean);
    u32 result = 0;
    f32 t = Uniform();
    while (t > g)
    {
        ++result;
        t *= Uniform();
    }
    return result;
}
