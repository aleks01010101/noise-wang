#include "BetterGradientNoise.hpp"

#include "generators/Interpolator.hpp"
#include "generators/NoiseCommon.hpp"

#include "image/ImageData.hpp"
#include "utility/Random.hpp"

#include <cassert>

static std::vector<f32> sGradientsX;
static std::vector<f32> sGradientsY;
static std::vector<u32> sPermutationsX;
static std::vector<u32> sPermutationsY;
static std::vector<u32> sPermutationsZ;

struct Hasher
{
    u32 operator ()(u32 x, u32 y, u32 z)
    {
        return sPermutationsX[x & 255] ^ sPermutationsY[y & 255] ^ sPermutationsZ[z & 255];
    }
};

struct WangWrap
{
    void operator()(u32& x, u32& y, u32 tw, u32 th, u32 lw, u32 lh)
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

static void initialize(u32 count, Random& rng, std::vector<u32>& permutations, std::vector<u32>& original)
{
    for (u32 i = 0; i < count; ++i)
        original.push_back(i);

    for (u32 i = 0; i < count; ++i)
    {
        u32 index = rng.Next() % original.size();
        permutations.push_back(original[index]);
        original[index] = original.back();
        original.pop_back();
    }

    for (u32 i = 0; i < count; ++i)
        permutations.push_back(permutations[i]);
}

template<class Interpolator>
void BetterGradientNoise<Interpolator>::EnsureInitialized()
{
    if (!sGradientsX.empty())
        return;

    static constexpr u32 kCount = 256;

    sGradientsX.reserve(kCount);
    sGradientsY.reserve(kCount);

    Random rand(2);

    for (u32 i = 0; i < kCount; ++i)
    {
        f32 x = rand.Uniform(-1.0f, 1.0f);
        f32 y = rand.Uniform(-1.0f, 1.0f);
        f32 z = rand.Uniform(-1.0f, 1.0f);
        f32 n = x * x + y * y + z * z;
        sGradientsX.push_back(x / n);
        sGradientsY.push_back(y / n);
    }

    sPermutationsX.reserve(kCount * 2);
    std::vector<u32> original;
    original.reserve(kCount);

    initialize(kCount, rand, sPermutationsX, original);
    initialize(kCount, rand, sPermutationsY, original);
    initialize(kCount, rand, sPermutationsZ, original);
}

template<class Interpolator>
void BetterGradientNoise<Interpolator>::Generate(const Parameters& parameters,
    const Lattice& latticeX, const Lattice& latticeY, ImageData& data)
{
    EnsureInitialized();

    Hasher hasher;

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
        u32 topIndex0 = maxLatticeY - latticeYStride;
        u32 topIndex1 = 0;
        u32 topIndex2 = latticeYStride;
        u32 topIndex3 = latticeYStride * 2;
        u32 yWeightIndex = 0;
        for (u32 y = 0; y < h; ++y)
        {
            const std::vector<u32>* xTops[] = {
                &latticeX[topIndex0],
                &latticeX[topIndex1],
                &latticeX[topIndex2],
                &latticeX[topIndex3]
            };
            const std::vector<u32>* yTops[] = {
                &latticeY[topIndex0],
                &latticeY[topIndex1],
                &latticeY[topIndex2],
                &latticeY[topIndex3]
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
                    const std::vector<u32>& xTop = *xTops[topIndex];
                    const std::vector<u32>& yTop = *yTops[topIndex];
                    const u32* xRow[] = {
                        &xTop[leftIndex0],
                        &xTop[leftIndex1],
                        &xTop[leftIndex2],
                        &xTop[leftIndex3]
                    };
                    const u32* yRow[] = {
                        &yTop[leftIndex0],
                        &yTop[leftIndex1],
                        &yTop[leftIndex2],
                        &yTop[leftIndex3]
                    };
                    u32 rowIndex = 0;
                    for (i32 i = -1; i < 3; ++i)
                    {
                        f32 dx = x0 - static_cast<f32>(i);

                        f32 dist = dx * dx + dy * dy;
                        if (dist < 4.0f)
                        {
                            u32 hash = hasher(*xRow[rowIndex], *yRow[rowIndex], 0);
                            f32 t = fmaf(dist, -0.25f, 1.0f);
                            f32 t2 = t * t;
                            f32 t4 = t2 * t2;
                            f32 poly = fmaf(t * t4, 4.0f, -t4 * 3.0f);

                            value += fmaf(dx, sGradientsX[hash], dy * sGradientsY[hash]) * poly;
                        }
                        ++rowIndex;
                    }
                    ++topIndex;
                }
                
                pixels[index] = fmaf(value, 0.5f, 0.5f);

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
void BetterGradientNoise<Interpolator>::GenerateSimple(const Parameters& parameters, ImageData& data)
{
    u32 w = data.GetWidth();
    u32 h = data.GetHeight();
    assert(w % parameters.latticeWidth == 0);
    assert(h % parameters.latticeHeight == 0);
    
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
        std::vector<u32>& rowX = latticeX[y];
        std::vector<u32>& rowY = latticeY[y];
        u32 j = y % parameters.latticeHeight;
        for (u32 x = 0; x < xPoints; ++x)
        {
            u32 i = x % parameters.latticeWidth;
            rowX[x] = i;
            rowY[x] = j;
        }
    }

    Generate(parameters, latticeX, latticeY, data);
}

template<class Interpolator>
void BetterGradientNoise<Interpolator>::GenerateWang(const Parameters& parameters, ImageData& data)
{
    u32 w = data.GetWidth();
    u32 h = data.GetHeight();
    assert(w % parameters.latticeWidth == 0);
    assert(h % parameters.latticeHeight == 0);
    assert((parameters.latticeWidth & 3) == 0);
    assert((parameters.latticeHeight & 3) == 0);
    
    u32 yPoints = parameters.latticeHeight + 1;
    u32 xPoints = parameters.latticeWidth + 1;
    Lattice latticeX(yPoints);
    Lattice latticeY(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
    {
        latticeX[i].resize(xPoints);
        latticeY[i].resize(xPoints);
    }

    WangWrap wrap;

    u32 lw = parameters.latticeWidth;
    u32 lh = parameters.latticeHeight;
    u32 tw = parameters.latticeWidth / 4;
    u32 th = parameters.latticeHeight / 4;
    
    for (u32 y = 0; y < yPoints; ++y)
    {
        std::vector<u32>& rowX = latticeX[y];
        std::vector<u32>& rowY = latticeY[y];
        for (u32 x = 0; x < xPoints; ++x)
        {
            u32 i = x;
            u32 j = y;
            wrap(i, j, tw, th, lw, lh);
            rowX[x] = i;
            rowY[x] = j;
        }
    }

    Generate(parameters, latticeX, latticeY, data);
}

template class BetterGradientNoise<FifthOrderInterpolator>;
