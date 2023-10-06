#pragma once

#include <cassert>

#include "ValueNoise.hpp"

#include "../../ImageData.hpp"
#include "../../Random.hpp"

template<class Interpolator>
ValueNoise<Interpolator>::ValueNoise()
{
}

template<class Interpolator>
ValueNoise<Interpolator>::~ValueNoise()
{
}

template<class Interpolator>
void ValueNoise<Interpolator>::Generate(ImageData& data, const std::vector<std::vector<f32>>& lattice) const
{
    const u32 maxLatticeY = static_cast<const u32>(lattice.size());
    const u32 maxLatticeX = static_cast<const u32>(lattice[0].size());
    std::vector<f32> xWeights;
    std::vector<f32> yWeights;

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
        u32 topIndex = 0;
        u32 bottomIndex = latticeYStride;
        u32 yWeightIndex = 0;
        for (u32 y = 0; y < h; ++y)
        {
            const std::vector<f32>& top = lattice[topIndex];
            const std::vector<f32>& bottom = lattice[bottomIndex];
            u32 xWeightIndex = 0;
            u32 leftIndex = 0;
            u32 rightIndex = latticeXStride;
            f32 yWeight = Interpolator()(yWeights[yWeightIndex]);
            f32 invYWeight = 1.0f - yWeight;
            for (u32 x = 0; x < w; ++x)
            {
                f32 tl = top[leftIndex];
                f32 tr = top[rightIndex];
                f32 bl = bottom[leftIndex];
                f32 br = bottom[rightIndex];

                f32 xWeight = Interpolator()(xWeights[xWeightIndex]);
                f32 invXWeight = 1.0f - xWeight;
                f32 t = tl * invXWeight + tr * xWeight;
                f32 b = bl * invXWeight + br * xWeight;
                pixels[index] = t * invYWeight + b * yWeight;

                ++index;
                ++xWeightIndex;
                if (xWeightIndex >= xWeightCount)
                {
                    xWeightIndex = 0;
                    leftIndex = rightIndex;
                    rightIndex += latticeXStride;
                    rightIndex = (rightIndex > maxLatticeX) ? maxLatticeX : rightIndex;
                }
            }
            ++yWeightIndex;
            if (yWeightIndex >= yWeightCount)
            {
                yWeightIndex = 0;
                topIndex = bottomIndex;
                bottomIndex += latticeYStride;
                bottomIndex = (bottomIndex > maxLatticeY) ? maxLatticeY : bottomIndex;
            }
        }
    }
}

template<class Interpolator>
void ValueNoise<Interpolator>::GenerateNoTiling(ImageData& data) const
{
    assert(m_Parameters.latticeWidth > 0);
    assert(m_Parameters.latticeHeight > 0);

    u32 yPoints = m_Parameters.latticeHeight + 1;
    u32 xPoints = m_Parameters.latticeWidth + 1;
    std::vector<std::vector<f32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);

    Random rand(1);

    for (u32 y = 0; y < yPoints; ++y)
    {
        std::vector<f32>& row = lattice[y];
        for (u32 x = 0; x < xPoints; ++x)
            row[x] = rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax);
    }

    Generate(data, lattice);
}

template<class Interpolator>
void ValueNoise<Interpolator>::GenerateSimple(ImageData& data) const
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
    std::vector<std::vector<f32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);

    Random rand(1);

    f32 corner = rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax);
    u32 yUnique = yPoints - 2;
    u32 xUnique = xPoints - 2;

    std::vector<f32>& top = lattice[0];
    std::vector<f32>& bottom = lattice[m_Parameters.latticeHeight];
    top[0] = corner;
    bottom[0] = corner;
    u32 index = 1;
    for (u32 x = 0; x < xUnique; ++x)
    {
        f32 value = rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax);
        top[index] = value;
        bottom[index] = value;
        ++index;
    }
    top[index] = corner;
    bottom[index] = corner;

    for (u32 y = 0; y < yUnique; ++y)
    {
        std::vector<f32>& row = lattice[y + 1];
        float border = rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax);
        row[0] = border;
        index = 1;
        for (u32 x = 0; x < xUnique; ++x)
        {
            row[index] = rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax);
            ++index;
        }
        row[index] = border;
    }

    Generate(data, lattice);
}

template<class Interpolator>
void ValueNoise<Interpolator>::GenerateWang(ImageData& data) const
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
    std::vector<std::vector<f32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);

    Random rand(1);

    f32 corner = rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax);
    u32 latticeTileWidth = m_Parameters.latticeWidth >> 2;
    u32 latticeTileHeight = m_Parameters.latticeHeight >> 2;
    u32 xTileUnique = latticeTileWidth - 1;
    u32 yTileUnique = latticeTileHeight - 1;

    // Fill in horizontal tile edges (red, green)
    u32 indices[] = {0, latticeTileHeight};
    const u32 tileCopyBytes = latticeTileWidth * sizeof(f32);
    for (u32 j = 0; j < 2; ++j)
    {
        std::vector<f32>& toFill = lattice[indices[j]];
        toFill[0] = corner;
        u32 index = 1;
        for (u32 i = 0; i < xTileUnique; ++i)
        {
            toFill[index++] = rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax);
        }
        toFill[index] = corner;
        f32* source = &toFill[1];
        f32* destination = source;
        // Copy the generated edge to all 4 tiles
        for (u32 i = 0; i < 3; ++i)
        {
            destination += latticeTileWidth;
            memcpy(destination, source, tileCopyBytes);
        }
    }
    
    // Copy generated rows to other rows that have same colors
    const u32 rowCopyBytes = xPoints * sizeof(f32);
    f32* source = &lattice[0][0];
    f32* destination = &lattice[latticeTileHeight * 3][0];
    memcpy(destination, source, rowCopyBytes);
    destination = &lattice[latticeTileHeight * 4][0];
    memcpy(destination, source, rowCopyBytes);
    
    source = &lattice[latticeTileHeight][0];
    destination = &lattice[latticeTileHeight * 2][0];
    memcpy(destination, source, rowCopyBytes);

    // Generate vertical tile edges
    std::vector<f32> vertical0(yTileUnique);
    std::vector<f32> vertical1(yTileUnique);
    for (u32 i = 0; i < yTileUnique; ++i)
    {
        vertical0[i] = rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax);
        vertical1[i] = rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax);
    }

    for (u32 verticalTileIndex = 0; verticalTileIndex < 4; ++verticalTileIndex)
    {
        u32 rowIndex = latticeTileHeight * verticalTileIndex + 1;
        for (u32 i = 0; i < yTileUnique; ++i)
        {
            std::vector<f32>& row = lattice[rowIndex++];
            row[0] = vertical0[i];
            row[latticeTileWidth] = vertical0[i];
            row[latticeTileWidth * 2] = vertical1[i];
            row[latticeTileWidth * 3] = vertical1[i];
            row[latticeTileWidth * 4] = vertical0[i];

            for (u32 horizontalTileIndex = 0; horizontalTileIndex < 4; ++horizontalTileIndex)
            {
                u32 index = horizontalTileIndex * latticeTileWidth + 1;
                for (u32 j = 0; j < xTileUnique; ++j)
                {
                    row[index++] = rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax);
                }
            }
        }
    }

    Generate(data, lattice);
}

template<class Interpolator>
void ValueNoise<Interpolator>::GenerateCorner(ImageData& data) const
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
    std::vector<std::vector<f32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);

    Random rand(1);

    f32 cornerR = rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax);
    f32 cornerG = rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax);
    u32 latticeTileWidth = m_Parameters.latticeWidth >> 2;
    u32 latticeTileHeight = m_Parameters.latticeHeight >> 2;
    u32 xTileUnique = latticeTileWidth - 1;
    u32 yTileUnique = latticeTileHeight - 1;

    f32 cornerValues[2] = {cornerR, cornerG};
    std::vector<f32> horizontalBorders[4];
    std::vector<f32> verticalBorders[4];

    for (u32 i = 0; i < 4; ++i)
    {
        horizontalBorders[i].reserve(xTileUnique);
        verticalBorders[i].reserve(yTileUnique);
        for (u32 j = 0; j < xTileUnique; ++j)
            horizontalBorders[i].push_back(rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax));
        for (u32 j = 0; j < yTileUnique; ++j)
            verticalBorders[i].push_back(rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax));
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

            const std::vector<f32>& topBorder = horizontalBorders[topBorderIndex];
            const std::vector<f32>& bottomBorder = horizontalBorders[bottomBorderIndex];
            const std::vector<f32>& leftBorder = verticalBorders[leftBorderIndex];
            const std::vector<f32>& rightBorder = verticalBorders[rightBorderIndex];

            u32 tileXOffset = latticeTileWidth * tileX;
            u32 tileYOffset = latticeTileHeight * tileY;
            lattice[tileYOffset][tileXOffset] = cornerValues[corners[tl]];
            memcpy(&lattice[tileYOffset][tileXOffset + 1], &topBorder[0], sizeof(f32) * xTileUnique);
            lattice[tileYOffset][tileXOffset + xTileUnique + 1] = cornerValues[corners[tr]];
            ++tileYOffset;
            for (u32 j = 0; j < yTileUnique; ++j)
            {
                u32 rowOffset = tileXOffset;
                std::vector<f32>& row = lattice[tileYOffset];
                row[rowOffset++] = leftBorder[j];
                for (u32 i = 0; i < xTileUnique; ++i)
                    row[rowOffset++] = rand.uniform(m_Parameters.rangeMin, m_Parameters.rangeMax);
                row[rowOffset] = rightBorder[j];
                ++tileYOffset;
            }
            lattice[tileYOffset][tileXOffset] = cornerValues[corners[bl]];
            memcpy(&lattice[tileYOffset][tileXOffset + 1], &bottomBorder[0], sizeof(f32) * xTileUnique);
            lattice[tileYOffset][tileXOffset + xTileUnique + 1] = cornerValues[corners[br]];
            
            ++tileIndex;
        }
        ++tileIndex;
    }

    Generate(data, lattice);
}
