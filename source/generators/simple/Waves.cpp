#include "Waves.hpp"

#include "image/ImageData.hpp"

#include <cassert>

void Waves::Generate(ImageData& data) const
{
    assert(data.GetChannelCount() == 1);

    u32 w = data.GetWidth();
    u32 h = data.GetHeight();

    f32 fh = static_cast<f32>(h);

    f32 frequency = parameters.frequency * 6.2832f / fh;

    f32* pixels = data.GetPixels(0);
    u32 counter = 0;
    for (u32 j = 0; j < h; ++j)
    {
        f32 fj = static_cast<f32>(j);
        f32 lineValue = fmaf(cosf(frequency * fj), 0.5f, 0.5f);
        for (u32 i = 0; i < w; ++i)
        {
            pixels[counter] = lineValue;
            ++counter;
        }
    }

    data.GenerateMips(0);
}

void Waves::GenerateNoTiling(ImageData& data) const
{
    Generate(data);
}

void Waves::GenerateSimple(ImageData& data) const
{
    Generate(data);
}

void Waves::GenerateWang(ImageData& data) const
{
    Generate(data);
}

void Waves::GenerateCorner(ImageData& data) const
{
    Generate(data);
}
