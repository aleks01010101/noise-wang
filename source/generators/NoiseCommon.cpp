#include "NoiseCommon.hpp"

#include <cmath>

void generateWeights(u32 count, std::vector<f32>& outWeights)
{
    outWeights.clear();
    if (count > 1)
    {
        f32 divisor = static_cast<f32>(count);
        for (u32 i = 0; i < count; ++i)
            outWeights.push_back(static_cast<f32>(i) / divisor);
    }
    else
        outWeights.push_back(0.5f);
}

f32 bilerp(f32 tl, f32 tr, f32 bl, f32 br, f32 yWeight, f32 invYWeight, f32 xWeight)
{
    f32 invXWeight = 1.0f - xWeight;
    f32 t = fmaf(tl, invXWeight, tr * xWeight);
    f32 b = fmaf(bl, invXWeight, br * xWeight);
    return fmaf(t, invYWeight, b * yWeight);
}
