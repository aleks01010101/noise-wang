#include "GaborNoise.hpp"

#include "IndexProviders.hpp"

#include "image/ImageData.hpp"
#include "utility/Random.hpp"

#include <cassert>

struct GaborKernel
{
    f32 operator ()(f32 width, f32 x, f32 y, f32 f0, f32 w0)
    {
        return expf(-width * fmaf(x, x, y * y)) * cosf(f0 * (fmaf(x, cosf(w0), y * sinf(w0))));
    }
};

template<class IndexProvider>
struct NoiseSampler
{
    f32 SampleCell(const IndexProvider& indexProvider, const GaborNoise::Parameters& parameters, i32 i, i32 j, f32 x, f32 y)
    {
        u32 index = parameters.cellOffset + indexProvider(i, j);
        Random generator(index);
        GaborKernel kernel;
        u32 impulseCount = generator.Poisson(static_cast<f32>(parameters.numberOfImpulsesPerCell));
        impulseCount = impulseCount > parameters.numberOfImpulsesPerCellCap ? parameters.numberOfImpulsesPerCellCap : impulseCount;
        f32 result = 0.0f;
        for (u32 k = 0; k < impulseCount; ++k)
        {
            f32 xi = generator.Uniform();
            f32 yi = generator.Uniform();
            f32 w = generator.Uniform(-1.0f, 1.0f);
            f32 o = generator.Uniform(parameters.frequencyOrientationMin, parameters.frequencyOrientationMax);
            f32 f = generator.Uniform(parameters.frequencyMagnitudeMin, parameters.frequencyMagnitudeMax);

            result += w * kernel(parameters.gaussianWidth, (x - xi) * parameters.cellSize, (y - yi) * parameters.cellSize, f, o);
        }

        return result;
    }

    f32 operator ()(const IndexProvider& indexProvider, const GaborNoise::Parameters& parameters, f32 x, f32 y)
    {
        f32 scaledX = x / parameters.cellSize;
        f32 scaledY = y / parameters.cellSize;
        f32 fx = scaledX - floorf(scaledX);
        f32 fy = scaledY - floorf(scaledY);
        i32 ix = static_cast<i32>(scaledX);
        i32 iy = static_cast<i32>(scaledY);

        f32 result = 0.0f;

        for (i32 j = -1; j < 2; ++j)
        {
            for (i32 i = -1; i < 2; ++i)
                result += SampleCell(indexProvider, parameters, ix + i, iy + j, fx - static_cast<f32>(i), fy - static_cast<f32>(j));
        }

        f32 value = fmaf(parameters.gaussianMagnitude * result, 0.5f, 0.5f);
        value = value > 1.0f ? 1.0f : value;
        value = value < 0.0f ? 0.0f : value;
        return value;
    }
};

template<class IndexProvider>
void GaborNoise::Generate(const Parameters& parameters, const IndexProvider& indexProvider, ImageData& data)
{
    NoiseSampler<IndexProvider> sampler;
    u32 mips = data.GetMipLevelCount();
    u32 width = data.GetWidth();
    u32 height = data.GetHeight();
    for (u32 mip = 0; mip < mips; ++mip)
    {
        u32 w;
        u32 h;
        data.GetDimensions(w, h, mip);

        f32 xScale = static_cast<f32>(width) / static_cast<f32>(w);
        f32 yScale = static_cast<f32>(height) / static_cast<f32>(h);

        f32* pixels = data.GetPixels(mip);

        u32 index = 0;
        for (u32 y = 0; y < h; ++y)
        {
            f32 fy = static_cast<f32>(y) * yScale;
            for (u32 x = 0; x < w; ++x)
            {
                pixels[index] = sampler(indexProvider, parameters, static_cast<f32>(x) * xScale, fy);
                ++index;
            }
        }
    }
}

void GaborNoise::GenerateSimple(const Parameters& parameters, ImageData& data)
{
    u32 width = data.GetWidth();
    SimpleTilingIndexProvider indexProvider(width / static_cast<u32>(parameters.cellSize));
    Generate(parameters, indexProvider, data);
}

void GaborNoise::GenerateWang(const Parameters& parameters, ImageData& data)
{
    u32 width = data.GetWidth();
    WangTilingIndexProvider indexProvider(width / static_cast<u32>(parameters.cellSize));
    Generate(parameters, indexProvider, data);
}
