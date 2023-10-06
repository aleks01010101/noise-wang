#include <cassert>

#include "GaborNoise.hpp"
#include "SpaceConvolutionIndexProvider.hpp"

#include "../../ImageData.hpp"
#include "../../Random.hpp"

static const f32 cPI = 3.1416f;
static const f32 cTwoPI = cPI * 2.0f;

struct GaborKernel
{
    inline f32 operator ()(const GaborNoise::Parameters& parameters, f32 x, f32 y, f32 f0, f32 w0)
    {
        return expf(-parameters.gaussianWidth * (x * x + y * y)) * cosf(f0 * (x * cosf(w0) + y * sinf(w0)));
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
        u32 impulseCount = generator.poisson(static_cast<f32>(parameters.numberOfImpulsesPerCell));
        impulseCount = impulseCount > parameters.numberOfImpulsesPerCellCap ? parameters.numberOfImpulsesPerCellCap : impulseCount;
        f32 result = 0.0f;
        for (u32 k = 0; k < impulseCount; ++k)
        {
            f32 xi = generator.uniform();
            f32 yi = generator.uniform();
            f32 w = generator.uniform(-1.0f, 1.0f);
            f32 o = generator.uniform(parameters.frequencyOrientationMin, parameters.frequencyOrientationMax);
            f32 f = generator.uniform(parameters.frequencyMagnitudeMin, parameters.frequencyMagnitudeMax);

            result += w * kernel(parameters, (x - xi) * parameters.cellSize, (y - yi) * parameters.cellSize, f, o);
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

        f32 value = parameters.gaussianMagnitude * result * 0.5f + 0.5f;
        value = value > 1.0f ? 1.0f : value;
        value = value < 0.0f ? 0.0f : value;
        return value;
    }
};

GaborNoise::GaborNoise()
{
}

GaborNoise::~GaborNoise()
{
}

template<class IndexProvider>
void GaborNoise::Generate(const IndexProvider& indexProvider, ImageData& data) const
{
    NoiseSampler<IndexProvider> sampler;
    u32 mips = data.GetMipLevelCount();
    u32 width;
    u32 height;
    data.GetDimensions(width, height, 0);
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
                pixels[index] = sampler(indexProvider, m_Parameters, static_cast<f32>(x) * xScale, fy);
                ++index;
            }
        }
    }
}

void GaborNoise::GenerateNoTiling(ImageData& data) const
{
    u32 width;
    u32 height;
    data.GetDimensions(width, height, 0);
    NoTilingIndexProvider indexProvider(width / static_cast<u32>(m_Parameters.cellSize));
    Generate(indexProvider, data);
}

void GaborNoise::GenerateSimple(ImageData& data) const
{
    u32 width;
    u32 height;
    data.GetDimensions(width, height, 0);
    SimpleTilingIndexProvider indexProvider(width / static_cast<u32>(m_Parameters.cellSize));
    Generate(indexProvider, data);
}

void GaborNoise::GenerateWang(ImageData& data) const
{
    u32 width;
    u32 height;
    data.GetDimensions(width, height, 0);
    WangTilingIndexProvider indexProvider(width / static_cast<u32>(m_Parameters.cellSize));
    Generate(indexProvider, data);
}

void GaborNoise::GenerateCorner(ImageData& data) const
{
    u32 width;
    u32 height;
    data.GetDimensions(width, height, 0);
    WangTilingIndexProvider indexProvider(width / static_cast<u32>(m_Parameters.cellSize));
    Generate(indexProvider, data);
}
