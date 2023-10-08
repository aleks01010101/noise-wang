#include "PerlinNoise.hpp"

#include "generators/Interpolator.hpp"
#include "image/ImageData.hpp"
#include "utility/Random.hpp"

#include <cassert>

static std::vector<f32> sGradientsX;
static std::vector<f32> sGradientsY;
static std::vector<u32> sPermutations;

template<class Interpolator>
void PerlinNoise<Interpolator>::EnsureInitialized()
{
    if (!sPermutations.empty())
        return;

    sGradientsX.reserve(16);
    sGradientsY.reserve(16);

    sGradientsX.push_back( 1.0f); sGradientsY.push_back( 1.0f);
    sGradientsX.push_back(-1.0f); sGradientsY.push_back( 1.0f);
    sGradientsX.push_back( 1.0f); sGradientsY.push_back(-1.0f);
    sGradientsX.push_back(-1.0f); sGradientsY.push_back(-1.0f);
    
    sGradientsX.push_back( 1.0f); sGradientsY.push_back(0.0f);
    sGradientsX.push_back(-1.0f); sGradientsY.push_back(0.0f);
    sGradientsX.push_back( 1.0f); sGradientsY.push_back(0.0f);
    sGradientsX.push_back(-1.0f); sGradientsY.push_back(0.0f);

    sGradientsX.push_back(0.0f); sGradientsY.push_back( 1.0f);
    sGradientsX.push_back(0.0f); sGradientsY.push_back(-1.0f);
    sGradientsX.push_back(0.0f); sGradientsY.push_back( 1.0f);
    sGradientsX.push_back(0.0f); sGradientsY.push_back(-1.0f);

    sGradientsX.push_back( 1.0f); sGradientsY.push_back( 1.0f);
    sGradientsX.push_back( 0.0f); sGradientsY.push_back(-1.0f);
    sGradientsX.push_back(-1.0f); sGradientsY.push_back( 1.0f);
    sGradientsX.push_back( 0.0f); sGradientsY.push_back(-1.0f);
    
    const u32 kCount = 256;
    sPermutations.resize(kCount * 2);
    std::vector<u32> original(kCount);
    for (u32 i = 0; i < kCount; ++i)
        original[i] = i;

    Random rand(1);

    for (u32 i = 0; i < kCount; ++i)
    {
        u32 index = rand.Next() % original.size();
        u32 value = original[index];
        sPermutations[i] = value;
        sPermutations[i + kCount] = value;
        original[index] = original.back();
        original.pop_back();
    }
}

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
void PerlinNoise<Interpolator>::Generate(const Lattice& latticeX, const Lattice& latticeY, const Parameters& parameters, ImageData& data)
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
void PerlinNoise<Interpolator>::GenerateSimple(const Parameters& parameters, ImageData& data)
{
    u32 w = data.GetWidth();
    u32 h = data.GetHeight();
    assert(w % parameters.latticeWidth == 0);
    assert(h % parameters.latticeHeight == 0);

    EnsureInitialized();

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
        u32 j = y % parameters.latticeHeight;
        for (u32 x = 0; x < xPoints; ++x)
        {
            u32 i = x % parameters.latticeWidth;
            u32 index = sPermutations[sPermutations[sPermutations[i] + j]] & 0xF;
            rowX[x] = sGradientsX[index];
            rowY[x] = sGradientsY[index];
        }
    }

    Generate(latticeX, latticeY, parameters, data);
}

struct TransformCoord
{
    void operator ()(u32& x, u32& y, u32 w, u32 h)
    {
        u32 tileX = x / w;
        u32 tileY = y / h;
        u32 inTileX = x - tileX * w;
        u32 inTileY = y - tileY * h;
    
        u32 l = (tileX / 2) & 1;
        u32 b = (tileY / 2) & 1;

        bool isXBorder = inTileX == 0;
        bool isYBorder = inTileY == 0;

        x = (isXBorder ? 0 : (isYBorder ? inTileX + b * w : x));
        y = (isYBorder ? 0 : (isXBorder ? inTileY + l * h : y));
    }
};

template<class Interpolator>
void PerlinNoise<Interpolator>::GenerateWang(const Parameters& parameters, ImageData& data)
{
    EnsureInitialized();

    std::vector<f32> xWeights;
    std::vector<f32> yWeights;

    u32 tileWidth = parameters.latticeWidth >> 2;
    u32 tileHeight = parameters.latticeHeight >> 2;

    u32 maxLatticeY = parameters.latticeHeight + 1;
    u32 maxLatticeX = parameters.latticeWidth + 1;

    TransformCoord transformer;

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
            u32 xWeightIndex = 0;
            u32 leftIndex = 0;
            u32 rightIndex = latticeXStride;
            f32 y0 = yWeights[yWeightIndex];
            f32 y1 = y0 - 1.0f;
            f32 yWeight = Interpolator()(y0);
            f32 invYWeight = 1.0f - yWeight;
            for (u32 x = 0; x < w; ++x)
            {
                u32 xtl = leftIndex;
                u32 ytl = topIndex;
                transformer(xtl, ytl, tileWidth, tileHeight);
                u32 gradientIndex = sPermutations[sPermutations[sPermutations[xtl] + ytl]] & 0xF;
                f32 tlX = sGradientsX[gradientIndex];
                f32 tlY = sGradientsY[gradientIndex];
                u32 xtr = rightIndex;
                u32 ytr = topIndex;
                transformer(xtr, ytr, tileWidth, tileHeight);
                gradientIndex = sPermutations[sPermutations[sPermutations[xtr] + ytr]] & 0xF;
                f32 trX = sGradientsX[gradientIndex];
                f32 trY = sGradientsY[gradientIndex];

                u32 xbl = leftIndex;
                u32 ybl = bottomIndex;
                transformer(xbl, ybl, tileWidth, tileHeight);
                gradientIndex = sPermutations[sPermutations[sPermutations[xbl] + ybl]] & 0xF;
                f32 blX = sGradientsX[gradientIndex];
                f32 blY = sGradientsY[gradientIndex];
                u32 xbr = rightIndex;
                u32 ybr = bottomIndex;
                transformer(xbr, ybr, tileWidth, tileHeight);
                gradientIndex = sPermutations[sPermutations[sPermutations[xbr] + ybr]] & 0xF;
                f32 brX = sGradientsX[gradientIndex];
                f32 brY = sGradientsY[gradientIndex];
                
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

template class PerlinNoise<FifthOrderInterpolator>;
