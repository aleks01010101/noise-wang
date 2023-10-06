#include <cassert>
#include <algorithm>

#include "WorleyNoise.hpp"
#include "SpaceConvolutionIndexProvider.hpp"

#include "../../ImageData.hpp"
#include "../../Random.hpp"
#include "../../Vector3.hpp"

WorleyNoise::WorleyNoise()
{
}

WorleyNoise::~WorleyNoise()
{
}

struct Result
{
    f32 f0;
    f32 f1;
    u32 i0;
    u32 i1;

    void update(f32 value, u32 parameter)
    {
        if (value < f0)
        {
            f1 = f0;
            i1 = i0;
            f0 = value;
            i0 = parameter;
        }
        else if (value < f1)
        {
            f1 = value;
            i1 = parameter;
        }
    }
};

template<class IndexProvider>
struct NoiseSampler
{
    void SampleCell(Result& result, const IndexProvider& indexProvider, const WorleyNoise::Parameters& parameters, i32 i, i32 j, f32 x, f32 y)
    {
        u32 index = parameters.cellIndexOffset + indexProvider(i, j);
        Random generator(index);

        u32 pointCount = generator.uniform(parameters.minPointsPerCell, parameters.maxPointsPerCell);

        for (u32 point = 0; point < pointCount; ++point)
        {
            f32 xi = x - generator.uniform();
            f32 yi = y - generator.uniform();

            result.update(xi * xi + yi * yi, index + point * parameters.cellsPerRow * parameters.cellsPerRow);
        }
    }

    vec3f32 operator ()(const IndexProvider& indexProvider, const WorleyNoise::Parameters& parameters, f32 x, f32 y)
    {
        f32 scaledX = x / parameters.cellSize;
        f32 scaledY = y / parameters.cellSize;
        f32 fx = scaledX - floorf(scaledX);
        f32 fy = scaledY - floorf(scaledY);
        i32 ix = static_cast<i32>(scaledX);
        i32 iy = static_cast<i32>(scaledY);

        Result result;
        result.f0 = parameters.cellSize * 2.0f;
        result.f0 *= result.f0;
        result.f1 = result.f0;

        for (i32 j = -1; j < 2; ++j)
        {
            for (i32 i = -1; i < 2; ++i)
                SampleCell(result, indexProvider, parameters, ix + i, iy + j, fx - static_cast<f32>(i), fy - static_cast<f32>(j));
        }

        Random generator(result.i0);
        float r = generator.uniform() * parameters.rMul + parameters.rAdd;
        float g = generator.uniform() * parameters.gMul + parameters.gAdd;
        float b = generator.uniform() * parameters.bMul + parameters.bAdd;

        vec3f32 color(r, g, b);
        color *= (1.0f - result.f0 / result.f1);

        return color;
    }
};

template<class IndexProvider>
void WorleyNoise::Generate(const IndexProvider& indexProvider, ImageData& data) const
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
                vec3f32 result = sampler(indexProvider, m_Parameters, static_cast<f32>(x) * xScale, fy);
                pixels[index++] = result.x;
                pixels[index++] = result.y;
                pixels[index++] = result.z;
                pixels[index++] = 1.0;
            }
        }
    }
}

void WorleyNoise::GenerateNoTiling(ImageData& data) const
{
    NoTilingIndexProvider indexProvider(m_Parameters.cellsPerRow);
    Generate(indexProvider, data);
}

void WorleyNoise::GenerateSimple(ImageData& data) const
{
    SimpleTilingIndexProvider indexProvider(m_Parameters.cellsPerRow);
    Generate(indexProvider, data);
}

void WorleyNoise::GenerateWang(ImageData& data) const
{
    WangTilingIndexProvider indexProvider(m_Parameters.cellsPerRow);
    Generate(indexProvider, data);
}

void WorleyNoise::GenerateCorner(ImageData& data) const
{
    WangTilingIndexProvider indexProvider(m_Parameters.cellsPerRow);
    Generate(indexProvider, data);
}
