#include "ModifiedNoise.hpp"

#include "generators/Interpolator.hpp"
#include "image/ImageData.hpp"

#include <cassert>

struct Hasher
{
    u32 operator ()(u32 value)
    {
        u32 temp = (value * value) % 61;
        temp = (temp * temp) % 61;
        return (temp * temp) % 61;
    }
};

struct Indexer
{
    u32 operator ()(Hasher& hasher, u32 x, u32 y)
    {
        return (hasher(y + hasher(x))) & 0x3;
    }
};

static void generateWeights(u32 count, std::vector<f32>& outWeights)
{
    outWeights.clear();
    if (count > 1)
    {
        f32 divisor = static_cast<f32>(count);
        for (u32 i = 0; i < count; ++i)
            outWeights.push_back(static_cast<f32>(i) / divisor);
    }
    else
        outWeights.push_back(0.5f);
}

template<class Interpolator>
void ModifiedNoise<Interpolator>::Generate(const Lattice& latticeX, const Lattice& latticeY, const Parameters& parameters, ImageData& data)
{
    const u32 maxLatticeY = static_cast<const u32>(latticeX.size());
    const u32 maxLatticeX = static_cast<const u32>(latticeX[0].size());
    std::vector<f32> xWeights;
    std::vector<f32> yWeights;

    const u32 mips = data.GetMipLevelCount();
    for (u32 mip = 0; mip < mips; ++mip)
    {
        u32 w;
        u32 h;
        data.GetDimensions(w, h, mip);

        u32 latticeXStride = parameters.latticeWidth / w;
        u32 latticeYStride = parameters.latticeHeight / h;
        latticeXStride = (latticeXStride >= 1) ? latticeXStride : 1;    
        latticeYStride = (latticeYStride >= 1) ? latticeYStride : 1;

        u32 xWeightCount = w / parameters.latticeWidth;
        u32 yWeightCount = h / parameters.latticeHeight;
        generateWeights(xWeightCount, xWeights);
        generateWeights(yWeightCount, yWeights);
        
        f32* pixels = data.GetPixels(mip);
        u32 index = 0;
        u32 topIndex = 0;
        u32 bottomIndex = latticeYStride;
        u32 yWeightIndex = 0;
        for (u32 y = 0; y < h; ++y)
        {
            const std::vector<f32>& topX = latticeX[topIndex];
            const std::vector<f32>& topY = latticeY[topIndex];
            const std::vector<f32>& bottomX = latticeX[bottomIndex];
            const std::vector<f32>& bottomY = latticeY[bottomIndex];
            u32 xWeightIndex = 0;
            u32 leftIndex = 0;
            u32 rightIndex = latticeXStride;
            f32 y0 = yWeights[yWeightIndex];
            f32 y1 = y0 - 1.0f;
            f32 yWeight = Interpolator()(y0);
            f32 invYWeight = 1.0f - yWeight;
            for (u32 x = 0; x < w; ++x)
            {
                f32 tlX = topX[leftIndex];
                f32 tlY = topY[leftIndex];
                f32 trX = topX[rightIndex];
                f32 trY = topY[rightIndex];
                f32 blX = bottomX[leftIndex];
                f32 blY = bottomY[leftIndex];
                f32 brX = bottomX[rightIndex];
                f32 brY = bottomY[rightIndex];
                
                f32 x0 = xWeights[xWeightIndex];
                f32 x1 = x0 - 1.0f;
                f32 xWeight = Interpolator()(x0);
                f32 invXWeight = 1.0f - xWeight;

                f32 g0 = fmaf(tlX, x0, tlY * y0);
                f32 g1 = fmaf(trX, x1, trY * y0);
                f32 g2 = fmaf(blX, x0, blY * y1);
                f32 g3 = fmaf(brX, x1, brY * y1);

                f32 t = fmaf(g0, invXWeight, g1 * xWeight);
                f32 b = fmaf(g2, invXWeight, g3 * xWeight);
                f32 value = fmaf(t, invYWeight, b * yWeight);
                pixels[index] = fmaf(value, 0.5f, 0.5f);

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
void ModifiedNoise<Interpolator>::GenerateSimple(const Parameters& parameters, ImageData& data)
{
    u32 w = data.GetWidth();
    u32 h = data.GetHeight();
    assert(w % parameters.latticeWidth == 0);
    assert(h % parameters.latticeHeight == 0);
    
    f32 gradientsX[] = {
        1.0f, -1.0f, 1.0f, -1.0f
    };
    f32 gradientsY[] = {
        1.0f, 1.0f, -1.0f, -1.0f
    };
    
    Hasher hasher;
    Indexer indexer;

    u32 yPoints = parameters.latticeHeight + 1;
    u32 xPoints = parameters.latticeWidth + 1;
    Lattice latticeX(yPoints);
    Lattice latticeY(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
    {
        latticeX[i].resize(xPoints);
        latticeY[i].resize(xPoints);
    }

    for (u32 y = 0; y < yPoints; ++y)
    {
        std::vector<f32>& rowX = latticeX[y];
        std::vector<f32>& rowY = latticeY[y];
        u32 j = (y % parameters.latticeHeight) + 1;
        for (u32 x = 0; x < xPoints; ++x)
        {
            u32 i = (x % parameters.latticeWidth) + 1;
            u32 index = indexer(hasher, i, j);
            rowX[x] = gradientsX[index];
            rowY[x] = gradientsY[index];
        }
    }

    Generate(latticeX, latticeY, parameters, data);
}

template<class Interpolator>
void ModifiedNoise<Interpolator>::GenerateWang(const Parameters& parameters, ImageData& data)
{
    u32 w = data.GetWidth();
    u32 h = data.GetHeight();
    assert(w % parameters.latticeWidth == 0);
    assert(h % parameters.latticeHeight == 0);
    assert((parameters.latticeWidth & 3) == 0);
    assert((parameters.latticeHeight & 3) == 0);

    Hasher hasher;
    Indexer indexer;

    f32 gradientsX[] = {
        1.0f, -1.0f, 1.0f, -1.0f
    };
    f32 gradientsY[] = {
        1.0f, 1.0f, -1.0f, -1.0f
    };

    u32 yPoints = parameters.latticeHeight + 1;
    u32 xPoints = parameters.latticeWidth + 1;
    Lattice latticeX(yPoints);
    Lattice latticeY(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
    {
        latticeX[i].resize(xPoints);
        latticeY[i].resize(xPoints);
    }
    
    u32 tileWidth = parameters.latticeWidth >> 2;
    u32 tileHeight = parameters.latticeHeight >> 2;
    u32 innerXPoints = tileWidth - 1;
    u32 innerYPoints = tileHeight - 1;

    u32 indices[] = {
        0, tileHeight
    };
    const u32 tileCopyBytes = tileWidth * sizeof(f32);
    u32 cornerIndex = indexer(hasher, 1, 1);
    f32 cornerX = gradientsX[cornerIndex];
    f32 cornerY = gradientsY[cornerIndex];
    for (u32 j = 0; j < 2; ++j)
    {
        std::vector<f32>& toFillX = latticeX[indices[j]];
        std::vector<f32>& toFillY = latticeY[indices[j]];
        toFillX[0] = cornerX;
        toFillY[0] = cornerY;
        u32 index = 1;
        for (u32 i = 0; i < innerXPoints; ++i)
        {
            u32 gradientIndex = indexer(hasher, index, j * tileHeight + 1);
            toFillX[index] = gradientsX[gradientIndex];
            toFillY[index] = gradientsY[gradientIndex];
            ++index;
        }
        toFillX[index] = cornerX;
        toFillY[index] = cornerY;
        f32* source = &toFillX[1];
        f32* destination = source;
        // Copy the generated edge to all 4 tiles
        for (u32 i = 0; i < 3; ++i)
        {
            destination += tileWidth;
            memcpy(destination, source, tileCopyBytes);
        }
        source = &toFillY[1];
        destination = source;
        // Copy the generated edge to all 4 tiles
        for (u32 i = 0; i < 3; ++i)
        {
            destination += tileWidth;
            memcpy(destination, source, tileCopyBytes);
        }
    }
    
    // Copy generated rows to other rows that have same colors
    const u32 rowCopyBytes = xPoints * sizeof(f32);
    f32* source = &latticeX[0][0];
    f32* destination = &latticeX[tileHeight * 3][0];
    memcpy(destination, source, rowCopyBytes);
    destination = &latticeX[tileHeight * 4][0];
    memcpy(destination, source, rowCopyBytes);
    
    source = &latticeX[tileHeight][0];
    destination = &latticeX[tileHeight * 2][0];
    memcpy(destination, source, rowCopyBytes);

    source = &latticeY[0][0];
    destination = &latticeY[tileHeight * 3][0];
    memcpy(destination, source, rowCopyBytes);
    destination = &latticeY[tileHeight * 4][0];
    memcpy(destination, source, rowCopyBytes);

    source = &latticeY[tileHeight][0];
    destination = &latticeY[tileHeight * 2][0];
    memcpy(destination, source, rowCopyBytes);

    // Generate vertical tile edges
    std::vector<f32> vertical0X(innerYPoints);
    std::vector<f32> vertical0Y(innerYPoints);
    std::vector<f32> vertical1X(innerYPoints);
    std::vector<f32> vertical1Y(innerYPoints);
    for (u32 i = 0; i < innerYPoints; ++i)
    {
        u32 index = indexer(hasher, 1, i + 1);
        vertical0X[i] = gradientsX[index];
        vertical0Y[i] = gradientsY[index];
        index = indexer(hasher, 1, i + tileHeight + 1);
        vertical1X[i] = gradientsX[index];
        vertical1Y[i] = gradientsY[index];
    }

    for (u32 verticalTileIndex = 0; verticalTileIndex < 4; ++verticalTileIndex)
    {
        u32 rowIndex = tileHeight * verticalTileIndex + 1;
        for (u32 i = 0; i < innerYPoints; ++i)
        {
            std::vector<f32>& rowX = latticeX[rowIndex];
            std::vector<f32>& rowY = latticeY[rowIndex];

            rowX[0] = vertical0X[i];
            rowY[0] = vertical0Y[i];
            rowX[tileWidth] = vertical0X[i];
            rowY[tileWidth] = vertical0Y[i];
            rowX[tileWidth * 2] = vertical1X[i];
            rowY[tileWidth * 2] = vertical1Y[i];
            rowX[tileWidth * 3] = vertical1X[i];
            rowY[tileWidth * 3] = vertical1Y[i];
            rowX[tileWidth * 4] = vertical0X[i];
            rowY[tileWidth * 4] = vertical0Y[i];

            for (u32 horizontalTileIndex = 0; horizontalTileIndex < 4; ++horizontalTileIndex)
            {
                u32 index = horizontalTileIndex * tileWidth + 1;
                for (u32 j = 0; j < innerXPoints; ++j)
                {
                    u32 gradientIndex = indexer(hasher, index + 1, rowIndex + 1);
                    rowX[index] = gradientsX[gradientIndex];
                    rowY[index] = gradientsY[gradientIndex];
                    ++index;
                }
            }

            ++rowIndex;
        }
    }

    Generate(latticeX, latticeY, parameters, data);
}

template class ModifiedNoise<FifthOrderInterpolator>;
