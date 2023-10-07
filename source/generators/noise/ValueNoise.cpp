#include "ValueNoise.hpp"

#include "generators/Interpolator.hpp"
#include "image/ImageData.hpp"
#include "utility/Random.hpp"

#include <cassert>

template<class Interpolator>
void ValueNoise<Interpolator>::Generate(TilingMode mode, const Parameters& parameters, ImageData& data)
{
    switch (mode)
    {
    case TilingMode::kSimple:
        GenerateSimple(parameters, data);
        break;
    case TilingMode::kWang:
        GenerateWang(parameters, data);
        break;
    }
}

template<class Interpolator>
void ValueNoise<Interpolator>::Generate(const std::vector<std::vector<f32>>& lattice, const Parameters& parameters, ImageData& data)
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

        u32 latticeXStride = parameters.latticeWidth / w;
        u32 latticeYStride = parameters.latticeHeight / h;
        latticeXStride = (latticeXStride >= 1) ? latticeXStride : 1;    
        latticeYStride = (latticeYStride >= 1) ? latticeYStride : 1;

        xWeights.clear();
        u32 xWeightCount = w / parameters.latticeWidth;
        if (xWeightCount <= 1)
            xWeights.push_back(0.5f);
        else
        {
            for (u32 i = 0; i < xWeightCount; ++i)
                xWeights.push_back(static_cast<f32>(i) / static_cast<f32>(xWeightCount));
        }

        yWeights.clear();
        u32 yWeightCount = h / parameters.latticeHeight;
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
void ValueNoise<Interpolator>::GenerateSimple(const Parameters& parameters, ImageData& data)
{
    u32 w;
    u32 h;
    data.GetDimensions(w, h, 0);
    assert(w % parameters.latticeWidth == 0);
    assert(h % parameters.latticeHeight == 0);
    assert(parameters.latticeWidth > 0);
    assert(parameters.latticeHeight > 0);

    u32 yPoints = parameters.latticeHeight + 1;
    u32 xPoints = parameters.latticeWidth + 1;
    std::vector<std::vector<f32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);

    Random rand(1);

    f32 corner = rand.Uniform(parameters.rangeMin, parameters.rangeMax);
    u32 yUnique = yPoints - 2;
    u32 xUnique = xPoints - 2;

    std::vector<f32>& top = lattice[0];
    std::vector<f32>& bottom = lattice[parameters.latticeHeight];
    top[0] = corner;
    bottom[0] = corner;
    u32 index = 1;
    for (u32 x = 0; x < xUnique; ++x)
    {
        f32 value = rand.Uniform(parameters.rangeMin, parameters.rangeMax);
        top[index] = value;
        bottom[index] = value;
        ++index;
    }
    top[index] = corner;
    bottom[index] = corner;

    for (u32 y = 0; y < yUnique; ++y)
    {
        std::vector<f32>& row = lattice[y + 1];
        float border = rand.Uniform(parameters.rangeMin, parameters.rangeMax);
        row[0] = border;
        index = 1;
        for (u32 x = 0; x < xUnique; ++x)
        {
            row[index] = rand.Uniform(parameters.rangeMin, parameters.rangeMax);
            ++index;
        }
        row[index] = border;
    }

    Generate(lattice, parameters, data);
}

template<class Interpolator>
void ValueNoise<Interpolator>::GenerateWang(const Parameters& parameters, ImageData& data)
{
    u32 w;
    u32 h;
    data.GetDimensions(w, h, 0);
    assert(w % parameters.latticeWidth == 0);
    assert(h % parameters.latticeHeight == 0);
    assert((parameters.latticeWidth & 3) == 0);
    assert((parameters.latticeHeight & 3) == 0);
    assert(parameters.latticeWidth > 0);
    assert(parameters.latticeHeight > 0);

    u32 yPoints = parameters.latticeHeight + 1;
    u32 xPoints = parameters.latticeWidth + 1;
    std::vector<std::vector<f32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);

    Random rand(1);

    f32 corner = rand.Uniform(parameters.rangeMin, parameters.rangeMax);
    u32 latticeTileWidth = parameters.latticeWidth >> 2;
    u32 latticeTileHeight = parameters.latticeHeight >> 2;
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
            toFill[index++] = rand.Uniform(parameters.rangeMin, parameters.rangeMax);
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
        vertical0[i] = rand.Uniform(parameters.rangeMin, parameters.rangeMax);
        vertical1[i] = rand.Uniform(parameters.rangeMin, parameters.rangeMax);
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
                    row[index++] = rand.Uniform(parameters.rangeMin, parameters.rangeMax);
                }
            }
        }
    }

    Generate(lattice, parameters, data);
}

template class ValueNoise<LinearInterpolator>;
