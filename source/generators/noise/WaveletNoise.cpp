#include "WaveletNoise.hpp"

#include "ValueNoise.cpp"

#include "generators/Interpolator.hpp"
#include "image/ImageData.hpp"

static constexpr u32 kRadius = 16;

static void Downsample(const f32* const source, f32* destination, u32 size, u32 stride)
{
    u32 length = size >> 1;
    f32 coefficients[] = {
            0.000334f, -0.001528f,  0.000410f,  0.003545f, -0.000938f, -0.008233f,  0.002172f,  0.019120f,
        -0.005040f, -0.044412f,  0.011655f,  0.103311f, -0.025936f, -0.243780f,  0.033979f,  0.655340f,
            0.655340f,  0.033979f, -0.243780f, -0.025936f,  0.103311f,  0.011655f, -0.044412f, -0.005040f,
            0.019120f,  0.002172f, -0.008233f, -0.000938f,  0.003546f,  0.000410f, -0.001528f,  0.000334f
    };

    const u32 radius = kRadius << 1;
    const u32 maxIndex = size * stride;
    for (u32 i = 0; i < length; ++i)
    {
        f32 value = 0.0f;
        u32 index = i * 2 - kRadius;
        index *= stride;
        for (u32 k = 0; k < radius; ++k)
        {
            value += coefficients[k] * source[index >= maxIndex ? 0 : index];
            index += stride;
        }

        destination[i * stride] = value;
    }
}

static inline void Upsample(const f32* const source, f32* destination, u32 size, u32 stride)
{
    f32 coefficients[] = {
        0.25f, 0.75f, 0.75f, 0.25f
    };

    const u32 maxIndex = (size >> 1) * stride;

    for (u32 i = 0; i < size; ++i)
    {
        f32 value = 0.0f;
        u32 base = i >> 1;
        u32 offset = i & 1;
        u32 index = base * stride;
        value += coefficients[offset] * source[index >= maxIndex ? 0 : index];
        index += stride;
        value += coefficients[offset + 2] * source[index >= maxIndex ? 0 : index];
        destination[i * stride] = value;
    }
}

template<class Interpolator>
void WaveletNoise<Interpolator>::Generate(const Parameters& parameters, ImageData& data, ImageData& baseNoise)
{
    u32 mipCount = data.GetMipLevelCount();

    std::vector<f32> downsampleBuffer;
    std::vector<f32> upsampleBuffer;

    const u32 maxLatticeY = parameters.latticeHeight;
    const u32 maxLatticeX = parameters.latticeWidth;
    std::vector<f32> xWeights;
    std::vector<f32> yWeights;

    u32 width = parameters.latticeWidth;
    u32 height = parameters.latticeHeight;

    u32 totalPixels = width * height;
    downsampleBuffer.clear();
    upsampleBuffer.clear();
    downsampleBuffer.resize(totalPixels);
    upsampleBuffer.resize(totalPixels);

    f32* baseSource = baseNoise.GetPixels(0);
    f32* source = baseSource;
    f32* downsample = &downsampleBuffer[0];
    f32* upsample = &upsampleBuffer[0];
    for (u32 i = 0; i < height; ++i)
    {
        Downsample(source, downsample, width, 1);
        Upsample(downsample, upsample, width, 1);

        source += width;
        downsample += width;
        upsample += width;
    }

    downsample = &downsampleBuffer[0];
    upsample = &upsampleBuffer[0];
    source = upsample;

    for (u32 i = 0; i < height; ++i)
    {
        Downsample(source, downsample, height, width);
        Upsample(downsample, upsample, height, width);

        ++source;
        ++downsample;
        ++upsample;
    }

    source = baseSource;
    upsample = &upsampleBuffer[0];
    for (u32 i = 0; i < totalPixels; ++i)
    {
        source[i] -= upsample[i];
    }

    for (u32 mip = 0; mip < mipCount; ++mip)
    {
        data.GetDimensions(width, height, mip);

        f32* pixels = data.GetPixels(mip);
        u32 latticeXStride = parameters.latticeWidth / width;
        u32 latticeYStride = parameters.latticeHeight / height;
        latticeXStride = (latticeXStride >= 1) ? latticeXStride : 1;    
        latticeYStride = (latticeYStride >= 1) ? latticeYStride : 1;

        if (width == latticeXStride || height == latticeYStride)
        {
            data.GenerateMips(mip - 1);
            break;
        }

        xWeights.clear();
        u32 xWeightCount = width / parameters.latticeWidth;
        if (xWeightCount <= 1)
            xWeights.push_back(0.5f);
        else
        {
            for (u32 i = 0; i < xWeightCount; ++i)
                xWeights.push_back(static_cast<f32>(i) / static_cast<f32>(xWeightCount));
        }

        yWeights.clear();
        u32 yWeightCount = height / parameters.latticeHeight;
        if (yWeightCount <= 1)
            yWeights.push_back(0.5f);
        else
        {
            for (u32 i = 0; i < yWeightCount; ++i)
                yWeights.push_back(static_cast<f32>(i) / static_cast<f32>(yWeightCount));
        }

        u32 index = 0;
        u32 topIndex = 0;
        u32 bottomIndex = latticeYStride;
        u32 yWeightIndex = 0;
        for (u32 j = 0; j < height; ++j)
        {
            const f32* top = &baseSource[topIndex * parameters.latticeWidth];
            const f32* bottom = &baseSource[bottomIndex * parameters.latticeWidth];

            u32 xWeightIndex = 0;
            u32 leftIndex = 0;
            u32 rightIndex = latticeXStride;
            f32 yWeight = Interpolator()(yWeights[yWeightIndex]);
            f32 invYWeight = 1.0f - yWeight;
            for (u32 i = 0; i < width; ++i)
            {
                f32 xWeight = Interpolator()(xWeights[xWeightIndex]);
                f32 invXWeight = 1.0f - xWeight;

                f32 tl = top[leftIndex];
                f32 tr = top[rightIndex];
                f32 bl = bottom[leftIndex];
                f32 br = bottom[rightIndex];

                f32 t = tl * invXWeight + tr * xWeight;
                f32 b = bl * invXWeight + br * xWeight;
                pixels[index] = (t * invYWeight + b * yWeight) * 0.5f + 0.5f;

                ++index;
                ++xWeightIndex;
                if (xWeightIndex >= xWeightCount)
                {
                    xWeightIndex = 0;
                    leftIndex = rightIndex;
                    rightIndex += latticeXStride;
                    rightIndex = (rightIndex >= maxLatticeX) ? 0 : rightIndex;
                }
            }

            ++yWeightIndex;
            if (yWeightIndex >= yWeightCount)
            {
                yWeightIndex = 0;
                topIndex = bottomIndex;
                bottomIndex += latticeYStride;
                bottomIndex = (bottomIndex >= maxLatticeY) ? 0 : bottomIndex;
            }
        }
    }
}

template<class Interpolator>
void WaveletNoise<Interpolator>::GenerateSimple(const Parameters& parameters, ImageData& data)
{
    ImageData baseNoise(parameters.latticeWidth, parameters.latticeHeight, 1, false);

    ValueNoise<LinearInterpolator>::Parameters p;
    p.rangeMin = -1.0f;
    p.rangeMax = 1.0f;
    p.latticeWidth = parameters.latticeWidth;
    p.latticeHeight = parameters.latticeHeight;
    ValueNoise<LinearInterpolator>::GenerateSimple(p, baseNoise);

    Generate(parameters, data, baseNoise);
}

template<class Interpolator>
void WaveletNoise<Interpolator>::GenerateWang(const Parameters& parameters, ImageData& data)
{
    ImageData baseNoise(parameters.latticeWidth, parameters.latticeHeight, 1, false);

    ValueNoise<LinearInterpolator>::Parameters p;
    p.rangeMin = -1.0f;
    p.rangeMax = 1.0f;
    p.latticeWidth = parameters.latticeWidth;
    p.latticeHeight = parameters.latticeHeight;
    ValueNoise<LinearInterpolator>::GenerateWang(p, baseNoise);

    Generate(parameters, data, baseNoise);
}

template class WaveletNoise<FifthOrderInterpolator>;
