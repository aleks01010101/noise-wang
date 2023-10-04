#include "ImageData.hpp"

#include "format/TGAFileFormat.hpp"

#include <cassert>
#include <cmath>
#include <sstream>

ImageData::ImageData(u32 width, u32 height, u32 channels, bool generateMipChain)
    : m_Width(width)
    , m_Height(height)
    , m_Channels(channels)
{
    assert(width > 0 && height > 0);

    bool first = true;
    while ((first || generateMipChain) && (width > 1 || height > 1))
    {
        u32 elementCount = channels * width * height;
        m_MipLevels.emplace_back(elementCount, 0.0f);

        first = false;
        width >>= 1;
        height >>= 1;
        width = width > 0 ? width : 1;
        height = height > 0 ? height : 1;
    }
    // Add last mip
    if (generateMipChain)
    {
        u32 elementCount = channels;
        m_MipLevels.emplace_back(elementCount, 0.0f);
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

void ImageData::GetDimensions(u32& width, u32& height, u32 mipLevel) const
{
    assert(mipLevel < GetMipLevelCount());
    width = m_Width >> mipLevel;
    height = m_Height >> mipLevel;

    width = width > 0 ? width : 1;
    height = height > 0 ? height : 1;
}

u32 ImageData::GetMipLevelCount() const
{
    return static_cast<u32>(m_MipLevels.size());
}

f32* ImageData::GetPixels(u32 mipLevel)
{
    assert(mipLevel < GetMipLevelCount());
    return &m_MipLevels[mipLevel].front();
}

const f32* ImageData::GetPixels(u32 mipLevel) const
{
    assert(mipLevel < GetMipLevelCount());
    return &m_MipLevels[mipLevel].front();
}

void ImageData::Save(const std::string& baseFileName) const
{
    u32 mipCount = GetMipLevelCount();
    if (mipCount > 1)
    {
        u32 width = m_Width;
        u32 height = m_Height;
        for (u32 i = 0; i < mipCount; ++i)
        {
            std::stringstream s;
            s << baseFileName << "_mip" << i << ".tga";
            TGAFileFormat::Save(GetPixels(i), width, height, m_Channels, s.str());

            width >>= 1;
            height >>= 1;
            width = width > 0 ? width : 1;
            height = height > 0 ? height : 1;
        }
    }
    else
    {
        TGAFileFormat::Save(GetPixels(0), m_Width, m_Height, m_Channels, baseFileName + ".tga");
    }
}
