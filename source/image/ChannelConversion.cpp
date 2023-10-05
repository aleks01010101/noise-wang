#include "ChannelConversion.hpp"
#include "ImageData.hpp"

#include <cassert>

typedef void (*ConverterFunction)(const f32*, f32*, u32);

static void RToRRR1(const f32* source, f32* destination, u32 /*unused*/)
{
    destination[0] = *source;
    destination[1] = *source;
    destination[2] = *source;
    destination[3] = 1.0f;
}

static void RGToRG01(const f32* source, f32* destination, u32 /*unused*/)
{
    destination[0] = source[0];
    destination[1] = source[1];
    destination[2] = 0.0f;
    destination[3] = 1.0f;
}

static void Copy(const f32* source, f32* destination, u32 numChannels)
{
    for (u32 i = 0; i < numChannels; ++i)
        destination[i] = source[i];
}

static void DoConversion(const ImageData& source, ImageData& destination, ConverterFunction converter)
{
    assert(destination.GetMipLevelCount() == source.GetMipLevelCount());

    const u32 mipCount = destination.GetMipLevelCount();
    const u32 destinationChannelCount = destination.GetChannelCount();
    const u32 sourceChannelCount = source.GetChannelCount();

    for (u32 mip = 0; mip < mipCount; ++mip)
    {
        u32 w;
        u32 h;
        destination.GetDimensions(w, h, mip);
        f32* destinationPixels = destination.GetPixels(mip);
        const f32* sourcePixels = source.GetPixels(mip);

        for (u32 y = 0; y < h; ++y)
        {
            for (u32 x = 0; x < w; ++x)
            {
                converter(sourcePixels, destinationPixels, destinationChannelCount);
                
                destinationPixels += destinationChannelCount;
                sourcePixels += sourceChannelCount;
            }
        }
    }
}

void ChannelConverter::Copy(const ImageData& source, ImageData& destination)
{
    assert(destination.GetChannelCount() == source.GetChannelCount());
    DoConversion(source, destination, ::Copy);
}

void ChannelConverter::RGToRG01(const ImageData& source, ImageData& destination)
{
    assert(destination.GetChannelCount() == 4);
    assert(source.GetChannelCount() == 2);
    DoConversion(source, destination, ::RGToRG01);
}

void ChannelConverter::RToRRR1(const ImageData& source, ImageData& destination)
{
    assert(destination.GetChannelCount() == 4);
    assert(source.GetChannelCount() == 1);
    DoConversion(source, destination, ::RToRRR1);
}
