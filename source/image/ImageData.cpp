#include "ImageData.hpp"

#include "format/TGAFileFormat.hpp"

#include <cassert>
#include <cmath>
#include <sstream>

ImageData::ImageData(u32 mip0Width, u32 mip0Height, u32 numChannels, bool generateMipChain)
    : width(mip0Width)
    , height(mip0Height)
    , channels(numChannels)
{
    assert(width > 0 && height > 0);

    u32 w = width;
    u32 h = height;

    bool first = true;
    while ((first || generateMipChain) && (w > 1 || h > 1))
    {
        u32 elementCount = channels * w * h;
        mipLevels.emplace_back(elementCount, 0.0f);

        first = false;
        w >>= 1;
        h >>= 1;
        w = w > 0 ? w : 1;
        h = h > 0 ? h : 1;
    }
    // Add last mip
    if (generateMipChain)
    {
        u32 elementCount = channels;
        mipLevels.emplace_back(elementCount, 0.0f);
    }
}

void ImageData::GenerateMips(u32 base)
{
    u32 mipLevelCount = GetMipLevelCount();
    for (u32 i = base + 1; i < mipLevelCount; ++i)
    {
        u32 sourceWidth;
        u32 sourceHeight;
        u32 destinationWidth;
        u32 destinationHeight;
        GetDimensions(sourceWidth, sourceHeight, i - 1);
        GetDimensions(destinationWidth, destinationHeight, i);

        u32 sourceIndex = 0;
        const u32 pixelSize = GetChannelCount();
        const u32 sourcePitch = sourceWidth * pixelSize;
        u32 nextSourceIndex = sourcePitch;
        u32 destinationIndex = 0;

        const f32* sourcePixels = GetPixels(i - 1);
        f32* destinationPixels = GetPixels(i);

        for (u32 y = 0; y < destinationHeight; ++y)
        {
            for (u32 x = 0; x < destinationWidth; ++x)
            {
                for (u32 c = 0; c < pixelSize; ++c)
                {
                    // Bilinear interpolation with 0.5 as weights:
                    // (p[x, y] + p[x + 1, y] + p[x, y + 1] + p[x + 1, y + 1]) * 0.25

                    f32 a = sourcePixels[sourceIndex] + sourcePixels[sourceIndex + pixelSize];
                    f32 b = sourcePixels[nextSourceIndex] + sourcePixels[nextSourceIndex + pixelSize];
                    destinationPixels[destinationIndex] = (a + b) * 0.25f;

                    ++sourceIndex;
                    ++nextSourceIndex;
                    ++destinationIndex;
                }
                sourceIndex += pixelSize;
                nextSourceIndex += pixelSize;
            }

            sourceIndex += sourcePitch;
            nextSourceIndex += sourcePitch;
        }
    }
}

void ImageData::GetDimensions(u32& outWidth, u32& outHeight, u32 mipLevel) const
{
    assert(mipLevel < GetMipLevelCount());
    outWidth = width >> mipLevel;
    outHeight = height >> mipLevel;

    outWidth = outWidth > 0 ? outWidth : 1;
    outHeight = outHeight > 0 ? outHeight : 1;
}

u32 ImageData::GetMipLevelCount() const
{
    return static_cast<u32>(mipLevels.size());
}

f32* ImageData::GetPixels(u32 mipLevel)
{
    assert(mipLevel < GetMipLevelCount());
    return &mipLevels[mipLevel].front();
}

const f32* ImageData::GetPixels(u32 mipLevel) const
{
    assert(mipLevel < GetMipLevelCount());
    return &mipLevels[mipLevel].front();
}

void ImageData::Save(const std::string& baseFileName) const
{
    u32 mipCount = GetMipLevelCount();
    if (mipCount > 1)
    {
        u32 w = width;
        u32 h = height;
        for (u32 i = 0; i < mipCount; ++i)
        {
            std::stringstream s;
            s << baseFileName << "_mip" << i << ".tga";
            TGAFileFormat::Save(GetPixels(i), w, h, channels, s.str());

            w >>= 1;
            h >>= 1;
            w = w > 0 ? w : 1;
            h = h > 0 ? h : 1;
        }
    }
    else
    {
        TGAFileFormat::Save(GetPixels(0), width, height, channels, baseFileName + ".tga");
    }
}
