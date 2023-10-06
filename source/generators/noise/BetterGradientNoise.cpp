#include <cassert>

#include "BetterGradientNoise.hpp"

#include "../../ImageData.hpp"
#include "../../Random.hpp"

namespace nBetterGradient
{
    static std::vector<vec3f32> s_Gradients;
    static std::vector<u32> s_PermutationsX;
    static std::vector<u32> s_PermutationsY;
    static std::vector<u32> s_PermutationsZ;

    struct Hasher
    {
        inline u32 operator ()(const vec3u32& coord)
        {
            return s_PermutationsX[coord.x & 255] ^ s_PermutationsY[coord.y & 255] ^ s_PermutationsZ[coord.z & 255];
        }

        inline u32 operator ()(u32 x, u32 y, u32 z)
        {
            return s_PermutationsX[x & 255] ^ s_PermutationsY[y & 255] ^ s_PermutationsZ[z & 255];
        }
    };

    struct WangWrap
    {
        inline void operator()(u32& x, u32& y, u32 tw, u32 th, u32 lw, u32 lh)
        {
            x = x % lw;
            y = y % lh;

            u32 inTileX = x % tw;
            u32 inTileY = y % th;
            u32 tileX = x / tw;
            u32 tileY = x / th;

            bool isX0Border = inTileX < 2;
            bool isX1Border = inTileX >= tw - 2;
            bool isXBorder = isX0Border || isX1Border;
            bool isY0Border = inTileY < 2;
            bool isY1Border = inTileY >= th - 2;
            bool isYBorder = isY0Border || isY1Border;
            bool isBorder = isXBorder || isYBorder;
            bool isCorner = isYBorder && isXBorder;

            u32 left = tileX / 2;
            u32 bottom = tileY / 2;
            u32 right = left != (tileX & 1);
            u32 top = bottom != (tileY & 1);

            u32 tileOffsetX = (isCorner || !isXBorder) ? 0 : (isX0Border ? left : right);
            u32 tileOffsetY = (isCorner || !isYBorder) ? 0 : (isY1Border ? bottom : top);

            x = isBorder ? tileOffsetX * tw + inTileX : x;
            y = isBorder ? tileOffsetY * th + inTileY : y;
        }
    };
}

template<class Interpolator>
void BetterGradientNoise<Interpolator>::Initialize()
{
    std::vector<vec3f32>& s_Gradients = nBetterGradient::s_Gradients;
    std::vector<u32>& s_PermutationsX = nBetterGradient::s_PermutationsX;
    std::vector<u32>& s_PermutationsY = nBetterGradient::s_PermutationsY;
    std::vector<u32>& s_PermutationsZ = nBetterGradient::s_PermutationsZ;

    if (s_Gradients.size() > 0)
        return;

    s_Gradients.reserve(256);

    Random rand(2);

    for (u32 i = 0; i < 256; ++i)
    {
        f32 x = rand.uniform(-1.0f, 1.0f);
        f32 y = rand.uniform(-1.0f, 1.0f);
        f32 z = rand.uniform(-1.0f, 1.0f);
        f32 n = x * x + y * y + z * z;
        s_Gradients.push_back(vec3f32(x / n, y / n, z / n));
    }

    s_PermutationsX.reserve(512);
    std::vector<u32> original;
    original.reserve(256);
    for (u32 i = 0; i < 256; ++i)
        original.push_back(i);

    for (u32 i = 0; i < 256; ++i)
    {
        u32 index = rand.next() % original.size();
        s_PermutationsX.push_back(original[index]);
        original[index] = original.back();
        original.pop_back();
    }

    for (u32 i = 0; i < 256; ++i)
        original.push_back(i);

    for (u32 i = 0; i < 256; ++i)
    {
        u32 index = rand.next() % original.size();
        s_PermutationsY.push_back(original[index]);
        original[index] = original.back();
        original.pop_back();
    }

    for (u32 i = 0; i < 256; ++i)
        original.push_back(i);

    for (u32 i = 0; i < 256; ++i)
    {
        u32 index = rand.next() % original.size();
        s_PermutationsZ.push_back(original[index]);
        original[index] = original.back();
        original.pop_back();
    }

    for (u32 i = 0; i < 256; ++i)
    {
        s_PermutationsX.push_back(s_PermutationsX[i]);
        s_PermutationsY.push_back(s_PermutationsY[i]);
        s_PermutationsZ.push_back(s_PermutationsZ[i]);
    }
}

template<class Interpolator>
BetterGradientNoise<Interpolator>::BetterGradientNoise()
{
}

template<class Interpolator>
BetterGradientNoise<Interpolator>::~BetterGradientNoise()
{
}

template<class Interpolator>
void BetterGradientNoise<Interpolator>::Generate(ImageData& data, const std::vector<std::vector<vec3u32>>& lattice) const
{
    Initialize();

    nBetterGradient::Hasher hasher;

    const u32 maxLatticeY = static_cast<const u32>(lattice.size());
    const u32 maxLatticeX = static_cast<const u32>(lattice[0].size());
    std::vector<f32> xWeights;
    std::vector<f32> yWeights;

    const std::vector<vec3f32>& s_Gradients = nBetterGradient::s_Gradients;

    const u32 mips = data.GetMipLevelCount();
    for (u32 mip = 0; mip < mips; ++mip)
    {
        u32 w;
        u32 h;
        data.GetDimensions(w, h, mip);

        u32 latticeXStride = m_Parameters.latticeWidth / w;
        u32 latticeYStride = m_Parameters.latticeHeight / h;
        latticeXStride = (latticeXStride >= 1) ? latticeXStride : 1;    
        latticeYStride = (latticeYStride >= 1) ? latticeYStride : 1;

        xWeights.clear();
        u32 xWeightCount = w / m_Parameters.latticeWidth;
        if (xWeightCount <= 1)
            xWeights.push_back(0.5f);
        else
        {
            for (u32 i = 0; i < xWeightCount; ++i)
                xWeights.push_back(static_cast<f32>(i) / static_cast<f32>(xWeightCount));
        }

        yWeights.clear();
        u32 yWeightCount = h / m_Parameters.latticeHeight;
        if (yWeightCount <= 1)
            yWeights.push_back(0.5f);
        else
        {
            for (u32 i = 0; i < yWeightCount; ++i)
                yWeights.push_back(static_cast<f32>(i) / static_cast<f32>(yWeightCount));
        }

        f32* pixels = data.GetPixels(mip);
        u32 index = 0;
        u32 topIndex0 = maxLatticeY - latticeYStride;
        u32 topIndex1 = 0;
        u32 topIndex2 = latticeYStride;
        u32 topIndex3 = latticeYStride * 2;
        u32 yWeightIndex = 0;
        for (u32 y = 0; y < h; ++y)
        {
            const std::vector<vec3u32>* tops[4] = {
                &lattice[topIndex0],
                &lattice[topIndex1],
                &lattice[topIndex2],
                &lattice[topIndex3]
            };
            u32 xWeightIndex = 0;
            u32 leftIndex0 = maxLatticeX - latticeXStride;
            u32 leftIndex1 = 0;
            u32 leftIndex2 = latticeXStride;
            u32 leftIndex3 = latticeXStride * 2;
            f32 y0 = yWeights[yWeightIndex];
            for (u32 x = 0; x < w; ++x)
            {
                f32 x0 = xWeights[xWeightIndex];
                f32 value = 0.0f;

                u32 topIndex = 0;
                for (i32 j = -1; j < 3; ++j)
                {
                    f32 dy = y0 - static_cast<f32>(j);
                    const std::vector<vec3u32>& top = *tops[topIndex];
                    const vec3u32* row[4] = {
                        &top[leftIndex0],
                        &top[leftIndex1],
                        &top[leftIndex2],
                        &top[leftIndex3]
                    };
                    u32 rowIndex = 0;
                    for (i32 i = -1; i < 3; ++i)
                    {
                        f32 dx = x0 - static_cast<f32>(i);

                        f32 dist = dx * dx + dy * dy;
                        if (dist < 4.0)
                        {
                            const vec3u32& coord = *row[rowIndex];
                            u32 hash = hasher(coord.x, coord.y, coord.z);
                            f32 t = 1.0f - dist * 0.25f;
                            f32 t2 = t * t;
                            f32 t4 = t2 * t2;
                            f32 poly = t * t4 * 4.0f - t4 * 3.0f;

                            const vec3f32& g = s_Gradients[hash];

                            value += (dx * g.x + dy * g.y) * poly;
                        }
                        ++rowIndex;
                    }
                    ++topIndex;
                }
                
                pixels[index] = (value + 1.0f) * 0.5f;

                ++index;
                ++xWeightIndex;
                if (xWeightIndex >= xWeightCount)
                {
                    xWeightIndex = 0;
                    leftIndex0 += latticeXStride;
                    leftIndex1 += latticeXStride;
                    leftIndex2 += latticeXStride;
                    leftIndex3 += latticeXStride;

                    if (leftIndex0 >= maxLatticeX)
                        leftIndex0 -= maxLatticeX;
                    if (leftIndex1 >= maxLatticeX)
                        leftIndex1 -= maxLatticeX;
                    if (leftIndex2 >= maxLatticeX)
                        leftIndex2 -= maxLatticeX;
                    if (leftIndex3 >= maxLatticeX)
                        leftIndex3 -= maxLatticeX;
                }
            }
            ++yWeightIndex;
            if (yWeightIndex >= yWeightCount)
            {
                yWeightIndex = 0;
                topIndex0 += latticeYStride;
                topIndex1 += latticeYStride;
                topIndex2 += latticeYStride;
                topIndex3 += latticeYStride;

                if (topIndex0 >= maxLatticeY)
                    topIndex0 -= maxLatticeY;
                if (topIndex1 >= maxLatticeY)
                    topIndex1 -= maxLatticeY;
                if (topIndex2 >= maxLatticeY)
                    topIndex2 -= maxLatticeY;
                if (topIndex3 >= maxLatticeY)
                    topIndex3 -= maxLatticeY;
            }
        }
    }
}

template<class Interpolator>
void BetterGradientNoise<Interpolator>::GenerateNoTiling(ImageData& data) const
{
    assert(m_Parameters.latticeWidth > 0);
    assert(m_Parameters.latticeHeight > 0);

    u32 yPoints = m_Parameters.latticeHeight + 1;
    u32 xPoints = m_Parameters.latticeWidth + 1;
    std::vector<std::vector<vec3u32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);
    
    for (u32 y = 0; y < yPoints; ++y)
    {
        std::vector<vec3u32>& row = lattice[y];
        for (u32 x = 0; x < xPoints; ++x)
        {
            row[x].x = x;
            row[x].y = y;
            row[x].z = 0;
        }
    }

    Generate(data, lattice);
}

template<class Interpolator>
void BetterGradientNoise<Interpolator>::GenerateSimple(ImageData& data) const
{
    u32 w;
    u32 h;
    data.GetDimensions(w, h, 0);
    assert(w % m_Parameters.latticeWidth == 0);
    assert(h % m_Parameters.latticeHeight == 0);
    assert(m_Parameters.latticeWidth > 0);
    assert(m_Parameters.latticeHeight > 0);

    u32 yPoints = m_Parameters.latticeHeight + 1;
    u32 xPoints = m_Parameters.latticeWidth + 1;
    std::vector<std::vector<vec3u32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);

    for (u32 y = 0; y < yPoints; ++y)
    {
        std::vector<vec3u32>& row = lattice[y];
        u32 j = y % m_Parameters.latticeHeight;
        for (u32 x = 0; x < xPoints; ++x)
        {
            u32 i = x % m_Parameters.latticeWidth;
            row[x].x = i;
            row[x].y = j;
            row[x].z = 0;
        }
    }

    Generate(data, lattice);
}

template<class Interpolator>
void BetterGradientNoise<Interpolator>::GenerateWang(ImageData& data) const
{
    u32 w;
    u32 h;
    data.GetDimensions(w, h, 0);
    assert(w % m_Parameters.latticeWidth == 0);
    assert(h % m_Parameters.latticeHeight == 0);
    assert((m_Parameters.latticeWidth & 3) == 0);
    assert((m_Parameters.latticeHeight & 3) == 0);
    assert(m_Parameters.latticeWidth > 0);
    assert(m_Parameters.latticeHeight > 0);

    u32 yPoints = m_Parameters.latticeHeight + 1;
    u32 xPoints = m_Parameters.latticeWidth + 1;
    std::vector<std::vector<vec3u32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);

    nBetterGradient::WangWrap wrap;

    u32 lw = m_Parameters.latticeWidth;
    u32 lh = m_Parameters.latticeHeight;
    u32 tw = m_Parameters.latticeWidth / 4;
    u32 th = m_Parameters.latticeHeight / 4;
    
    for (u32 y = 0; y < yPoints; ++y)
    {
        std::vector<vec3u32>& row = lattice[y];
        for (u32 x = 0; x < xPoints; ++x)
        {
            u32 i = x;
            u32 j = y;
            wrap(i, j, tw, th, lw, lh);
            row[x].x = i;
            row[x].y = j;
            row[x].z = 0;
        }
    }

    Generate(data, lattice);
}

template<class Interpolator>
void BetterGradientNoise<Interpolator>::GenerateCorner(ImageData& data) const
{
    u32 w;
    u32 h;
    data.GetDimensions(w, h, 0);
    assert(w % m_Parameters.latticeWidth == 0);
    assert(h % m_Parameters.latticeHeight == 0);
    assert((m_Parameters.latticeWidth & 3) == 0);
    assert((m_Parameters.latticeHeight & 3) == 0);
    assert(m_Parameters.latticeWidth > 0);
    assert(m_Parameters.latticeHeight > 0);

    Initialize();

    u32 yPoints = m_Parameters.latticeHeight + 1;
    u32 xPoints = m_Parameters.latticeWidth + 1;
    std::vector<std::vector<vec3u32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);

    u32 latticeTileWidth = m_Parameters.latticeWidth >> 2;
    u32 latticeTileHeight = m_Parameters.latticeHeight >> 2;
    u32 xTileUnique = latticeTileWidth - 1;
    u32 yTileUnique = latticeTileHeight - 1;

    vec3u32 cornerR = vec3u32(0, 0, 0);
    vec3u32 cornerG = vec3u32(0, 0, 1);
    vec3u32 cornerValues[2] = {cornerR, cornerG};
    std::vector<vec3u32> horizontalBorders[4];
    std::vector<vec3u32> verticalBorders[4];

    for (u32 i = 0; i < 4; ++i)
    {
        horizontalBorders[i].reserve(xTileUnique);
        verticalBorders[i].reserve(yTileUnique);
        for (u32 j = 0; j < xTileUnique; ++j)
            horizontalBorders[i].push_back(vec3u32(j + 1, 0, 0));
        for (u32 j = 0; j < yTileUnique; ++j)
            verticalBorders[i].push_back(vec3u32(0, j + 1, 0));
    }

    u8 corners[25] = {
        0, 0, 1, 0, 0,
        1, 0, 1, 1, 1,
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 0, 1, 0, 0
    };

    u32 tileIndex = 0;
    for (u32 tileY = 0; tileY < 4; ++tileY)
    {
        for (u32 tileX = 0; tileX < 4; ++tileX)
        {
            u8 tl = corners[tileIndex];
            u8 tr = corners[tileIndex + 1];
            u8 bl = corners[tileIndex + 5];
            u8 br = corners[tileIndex + 6];

            u8 topBorderIndex = tl * 2 + tr;
            u8 bottomBorderIndex = bl * 2 + br;
            u8 leftBorderIndex = tl * 2 + bl;
            u8 rightBorderIndex = tr * 2 + br;

            const std::vector<vec3u32>& topBorder = horizontalBorders[topBorderIndex];
            const std::vector<vec3u32>& bottomBorder = horizontalBorders[bottomBorderIndex];
            const std::vector<vec3u32>& leftBorder = verticalBorders[leftBorderIndex];
            const std::vector<vec3u32>& rightBorder = verticalBorders[rightBorderIndex];

            u32 tileXOffset = latticeTileWidth * tileX;
            u32 tileYOffset = latticeTileHeight * tileY;
            lattice[tileYOffset][tileXOffset] = cornerValues[corners[tl]];
            memcpy(&lattice[tileYOffset][tileXOffset + 1], &topBorder[0], sizeof(vec3u32) * xTileUnique);
            lattice[tileYOffset][tileXOffset + xTileUnique + 1] = cornerValues[corners[tr]];
            ++tileYOffset;
            for (u32 j = 0; j < yTileUnique; ++j)
            {
                u32 rowOffset = tileXOffset;
                std::vector<vec3u32>& row = lattice[tileYOffset];
                row[rowOffset++] = leftBorder[j];
                for (u32 i = 0; i < xTileUnique; ++i)
                    row[rowOffset++] = vec3u32(i + tileXOffset + 1, j + tileYOffset + 1, 0);
                row[rowOffset] = rightBorder[j];
                ++tileYOffset;
            }
            lattice[tileYOffset][tileXOffset] = cornerValues[corners[bl]];
            memcpy(&lattice[tileYOffset][tileXOffset + 1], &bottomBorder[0], sizeof(vec3u32) * xTileUnique);
            lattice[tileYOffset][tileXOffset + xTileUnique + 1] = cornerValues[corners[br]];
            
            ++tileIndex;
        }
        ++tileIndex;
    }

    Generate(data, lattice);
}
