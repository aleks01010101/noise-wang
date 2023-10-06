#include "Checker.hpp"

#include "image/ImageData.hpp"
#include "utility/Random.hpp"

#include <cassert>
#include <vector>

void Checker::Generate(ImageData& data) const
{
    u32 mips = data.GetMipLevelCount();

    u32 w = data.GetWidth();
    u32 h = data.GetHeight();
    u32 tileWidth = parameters.tileWidth;
    u32 tileHeight = parameters.tileHeight;
    u32 xTiles = w / tileWidth;
    u32 yTiles = h / tileHeight;
    xTiles = (xTiles * tileWidth != w) ? (xTiles + 1) : xTiles;
    yTiles = (yTiles * tileHeight != h) ? (yTiles + 1) : yTiles;

    Random rand(1);

    std::vector<f32> tiles;
    tiles.reserve(xTiles * yTiles);
    bool isDark = false;
    for (u32 y = 0; y < yTiles; ++y)
    {
        bool firstWasDark = isDark;
        for (u32 x = 0; x < xTiles; ++x)
        {
            f32 low = isDark ? parameters.darkMin : parameters.brightMin;
            f32 high = isDark ? parameters.darkMax: parameters.brightMax;
            tiles.push_back(rand.Uniform(low, high));
            isDark = !isDark;
        }
        isDark = !firstWasDark;
    }

    for (u32 mip = 0; mip < mips; ++mip)
    {
        data.GetDimensions(w, h, mip);
        tileWidth = parameters.tileWidth >> mip;
        tileHeight = parameters.tileHeight >> mip;

        f32* pixels = data.GetPixels(mip);
        u32 pixelSize = data.GetChannelCount();

        if (tileWidth > 0 && tileHeight > 0)
        {
            u32 yCounter = 0;
            u32 tileIndex = 0;

            for (u32 j = 0; j < h; ++j)
            {
                u32 xCounter = 0;
                u32 tile = tileIndex;
                for (u32 i = 0; i < w; ++i)
                {
                    *pixels = tiles[tileIndex];
                    pixels += pixelSize;
                    ++xCounter;
                    if (xCounter == tileWidth)
                    {
                        xCounter = 0;
                        ++tileIndex;
                    }
                }

                ++yCounter;
                if (yCounter == tileHeight)
                    yCounter = 0;
                else
                    tileIndex = tile;
            }
        }
        else
        {
            data.GenerateMips(mip - 1);
            break;
        }
    }
}

void Checker::GenerateNoTiling(ImageData& data) const
{
    Generate(data);
}

void Checker::GenerateSimple(ImageData& data) const
{
    u32 w = data.GetWidth();
    u32 h = data.GetHeight();
    assert(w % parameters.tileWidth == 0);
    assert(h % parameters.tileHeight == 0);
    Generate(data);
}

void Checker::GenerateWang(ImageData& data) const
{
    u32 w = data.GetWidth();
    u32 h = data.GetHeight();
    w >>= 2;
    h >>= 2;
    assert(w % parameters.tileWidth == 0);
    assert(h % parameters.tileHeight == 0);
    Generate(data);
}

void Checker::GenerateCorner(ImageData& data) const
{
    u32 w = data.GetWidth();
    u32 h = data.GetHeight();
    w >>= 2;
    h >>= 2;
    assert(w % parameters.tileWidth == 0);
    assert(h % parameters.tileHeight == 0);
    Generate(data);
}
