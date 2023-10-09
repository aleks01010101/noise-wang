#pragma once

#include "utility/Types.hpp"

#include <cmath>

struct LinearInterpolator
{
    f32 operator ()(f32 weight)
    {
        return weight;
    }
};

struct CosineInterpolator
{
    f32 operator ()(f32 weight)
    {
        return fmaf(cosf(weight * 3.1416f), -0.5f, 0.5f);
    }
};

struct SmoothstepInterpolator
{
    f32 operator ()(f32 weight)
    {
        return weight * weight * fmaf(weight, -2.0f, 3.0f);
    }
};

struct FifthOrderInterpolator
{
    f32 operator ()(f32 weight)
    {
        return fmaf(fmaf(weight, 6.0f, -15.0f), weight, 10.0f) * weight * weight * weight;
    }
};
