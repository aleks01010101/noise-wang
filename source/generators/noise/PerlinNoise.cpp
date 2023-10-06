#include <cassert>

#include "PerlinNoise.hpp"

#include "../../ImageData.hpp"
#include "../../Random.hpp"

namespace nPerlin
{
    static std::vector<vec2f32> s_Gradients;
    static std::vector<u32> s_Permutations;
}

template<class Interpolator>
void PerlinNoise<Interpolator>::Initialize()
{
    std::vector<vec2f32>& s_Gradients = nPerlin::s_Gradients;
    std::vector<u32>& s_Permutations = nPerlin::s_Permutations;

    if (s_Permutations.size() > 0)
        return;

    s_Gradients.reserve(16);

    s_Gradients.push_back(vec2f32( 1.0f,  1.0f));
    s_Gradients.push_back(vec2f32(-1.0f,  1.0f));
    s_Gradients.push_back(vec2f32( 1.0f, -1.0f));
    s_Gradients.push_back(vec2f32(-1.0f, -1.0f));

    s_Gradients.push_back(vec2f32( 1.0f,  0.0f));
    s_Gradients.push_back(vec2f32(-1.0f,  0.0f));
    s_Gradients.push_back(vec2f32( 1.0f,  0.0f));
    s_Gradients.push_back(vec2f32(-1.0f,  0.0f));

    s_Gradients.push_back(vec2f32( 0.0f,  1.0f));
    s_Gradients.push_back(vec2f32( 0.0f, -1.0f));
    s_Gradients.push_back(vec2f32( 0.0f,  1.0f));
    s_Gradients.push_back(vec2f32( 0.0f, -1.0f));

    s_Gradients.push_back(vec2f32( 1.0f,  1.0f));
    s_Gradients.push_back(vec2f32( 0.0f, -1.0f));
    s_Gradients.push_back(vec2f32(-1.0f,  1.0f));
    s_Gradients.push_back(vec2f32( 0.0f, -1.0f));

    s_Permutations.reserve(512);
    std::vector<u32> original(256);
    for (u32 i = 0; i < 256; ++i)
        original[i] = i;

    Random rand(1);

    for (u32 i = 0; i < 256; ++i)
    {
        u32 index = rand.next() % original.size();
        s_Permutations.push_back(original[index]);
        original[index] = original.back();
        original.pop_back();
    }

    for (u32 i = 0; i < 256; ++i)
        s_Permutations.push_back(s_Permutations[i]);
}

template<class Interpolator>
PerlinNoise<Interpolator>::PerlinNoise()
{
}

template<class Interpolator>
PerlinNoise<Interpolator>::~PerlinNoise()
{
}

template<class Interpolator>
void PerlinNoise<Interpolator>::Generate(ImageData& data, const std::vector<std::vector<vec2f32>>& lattice) const
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
            const std::vector<vec2f32>& top = lattice[topIndex];
            const std::vector<vec2f32>& bottom = lattice[bottomIndex];
            u32 xWeightIndex = 0;
            u32 leftIndex = 0;
            u32 rightIndex = latticeXStride;
            f32 y0 = yWeights[yWeightIndex];
            f32 y1 = y0 - 1.0f;
            f32 yWeight = Interpolator()(y0);
            f32 invYWeight = 1.0f - yWeight;
            for (u32 x = 0; x < w; ++x)
            {
                const vec2f32& tl = top[leftIndex];
                const vec2f32& tr = top[rightIndex];
                const vec2f32& bl = bottom[leftIndex];
                const vec2f32& br = bottom[rightIndex];
                
                f32 x0 = xWeights[xWeightIndex];
                f32 x1 = x0 - 1.0f;
                f32 xWeight = Interpolator()(x0);
                f32 invXWeight = 1.0f - xWeight;

                f32 g0 = tl.x * x0 + tl.y * y0;
                f32 g1 = tr.x * x1 + tr.y * y0;
                f32 g2 = bl.x * x0 + bl.y * y1;
                f32 g3 = br.x * x1 + br.y * y1;

                f32 t = g0 * invXWeight + g1 * xWeight;
                f32 b = g2 * invXWeight + g3 * xWeight;
                f32 value = t * invYWeight + b * yWeight;
                pixels[index] = (value + 1.0f) * 0.5f;

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
void PerlinNoise<Interpolator>::GenerateNoTiling(ImageData& data) const
{
    assert(m_Parameters.latticeWidth > 0);
    assert(m_Parameters.latticeHeight > 0);

    Initialize();

    const std::vector<vec2f32>& s_Gradients = nPerlin::s_Gradients;
    const std::vector<u32>& s_Permutations = nPerlin::s_Permutations;

    /*

    u32 yPoints = m_Parameters.latticeHeight + 1;
    u32 xPoints = m_Parameters.latticeWidth + 1;
    std::vector<std::vector<vec2f32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);
    
    for (u32 y = 0; y < yPoints; ++y)
    {
        std::vector<vec2f32>& row = lattice[y];
        for (u32 x = 0; x < xPoints; ++x)
        {
            u32 index = s_Permutations[s_Permutations[s_Permutations[x] + y]] & 0xF;
            row[x] = s_Gradients[index];
        }
    }

    Generate(data, lattice);
    */

    std::vector<f32> xWeights;
    std::vector<f32> yWeights;

    u32 maxLatticeY = m_Parameters.latticeHeight + 1;
    u32 maxLatticeX = m_Parameters.latticeWidth + 1;

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
                const vec2f32& tl = s_Gradients[s_Permutations[s_Permutations[(s_Permutations[xtl & 0x1FF] + ytl) & 0x1FF]] & 0xF];
                u32 xtr = rightIndex;
                u32 ytr = topIndex;
                const vec2f32& tr = s_Gradients[s_Permutations[s_Permutations[(s_Permutations[xtr & 0x1FF] + ytr) & 0x1FF]] & 0xF];

                u32 xbl = leftIndex;
                u32 ybl = bottomIndex;
                const vec2f32& bl = s_Gradients[s_Permutations[s_Permutations[(s_Permutations[xbl & 0x1FF] + ybl) & 0x1FF]] & 0xF];
                u32 xbr = rightIndex;
                u32 ybr = bottomIndex;
                const vec2f32& br = s_Gradients[s_Permutations[s_Permutations[(s_Permutations[xbr & 0x1FF] + ybr) & 0x1FF]] & 0xF];
                
                f32 x0 = xWeights[xWeightIndex];
                f32 x1 = x0 - 1.0f;
                f32 xWeight = Interpolator()(x0);
                f32 invXWeight = 1.0f - xWeight;

                f32 g0 = tl.x * x0 + tl.y * y0;
                f32 g1 = tr.x * x1 + tr.y * y0;
                f32 g2 = bl.x * x0 + bl.y * y1;
                f32 g3 = br.x * x1 + br.y * y1;

                f32 t = g0 * invXWeight + g1 * xWeight;
                f32 b = g2 * invXWeight + g3 * xWeight;
                f32 value = t * invYWeight + b * yWeight;
                pixels[index] = (value + 1.0f) * 0.5f;

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
void PerlinNoise<Interpolator>::GenerateSimple(ImageData& data) const
{
    u32 w;
    u32 h;
    data.GetDimensions(w, h, 0);
    assert(w % m_Parameters.latticeWidth == 0);
    assert(h % m_Parameters.latticeHeight == 0);
    assert(m_Parameters.latticeWidth > 0);
    assert(m_Parameters.latticeHeight > 0);

    Initialize();

    const std::vector<vec2f32>& s_Gradients = nPerlin::s_Gradients;
    const std::vector<u32>& s_Permutations = nPerlin::s_Permutations;

    u32 yPoints = m_Parameters.latticeHeight + 1;
    u32 xPoints = m_Parameters.latticeWidth + 1;
    std::vector<std::vector<vec2f32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);

    for (u32 y = 0; y < yPoints; ++y)
    {
        std::vector<vec2f32>& row = lattice[y];
        u32 j = y % m_Parameters.latticeHeight;
        for (u32 x = 0; x < xPoints; ++x)
        {
            u32 i = x % m_Parameters.latticeWidth;
            u32 index = s_Permutations[s_Permutations[s_Permutations[i] + j]] & 0xF;
            row[x] = s_Gradients[index];
        }
    }

    Generate(data, lattice);
}

struct TransformCoord
{
    inline void operator ()(u32& x, u32& y, u32 w, u32 h)
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
void PerlinNoise<Interpolator>::GenerateWang(ImageData& data) const
{
    /*
    u32 w;
    u32 h;
    data.GetDimensions(w, h, 0);
    assert(w % m_Parameters.latticeWidth == 0);
    assert(h % m_Parameters.latticeHeight == 0);
    assert((m_Parameters.latticeWidth & 3) == 0);
    assert((m_Parameters.latticeHeight & 3) == 0);
    assert(m_Parameters.latticeWidth > 0);
    assert(m_Parameters.latticeHeight > 0);
    */

    Initialize();

    const std::vector<vec2f32>& s_Gradients = nPerlin::s_Gradients;
    const std::vector<u32>& s_Permutations = nPerlin::s_Permutations;
    /*
    u32 yPoints = m_Parameters.latticeHeight + 1;
    u32 xPoints = m_Parameters.latticeWidth + 1;
    std::vector<std::vector<vec2f32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);

    
    u32 tileWidth = m_Parameters.latticeWidth >> 2;
    u32 tileHeight = m_Parameters.latticeHeight >> 2;
    u32 innerXPoints = tileWidth - 1;
    u32 innerYPoints = tileHeight - 1;

    u32 indices[] = {0, tileHeight};
    const u32 tileCopyBytes = tileWidth * sizeof(vec2f32);
    vec2f32 corner = s_Gradients[s_Permutations[s_Permutations[s_Permutations[0]]] & 0xF];
    for (u32 j = 0; j < 2; ++j)
    {
        std::vector<vec2f32>& toFill = lattice[indices[j]];
        toFill[0] = corner;
        u32 index = 1;
        for (u32 i = 0; i < innerXPoints; ++i)
        {
            toFill[index] = s_Gradients[s_Permutations[s_Permutations[s_Permutations[index] + j * tileHeight]] & 0xF];
            ++index;
        }
        toFill[index] = corner;
        vec2f32* source = &toFill[1];
        vec2f32* destination = source;
        // Copy the generated edge to all 4 tiles
        for (u32 i = 0; i < 3; ++i)
        {
            destination += tileWidth;
            memcpy(destination, source, tileCopyBytes);
        }
    }
    
    // Copy generated rows to other rows that have same colors
    const u32 rowCopyBytes = xPoints * sizeof(vec2f32);
    vec2f32* source = &lattice[0][0];
    vec2f32* destination = &lattice[tileHeight * 3][0];
    memcpy(destination, source, rowCopyBytes);
    destination = &lattice[tileHeight * 4][0];
    memcpy(destination, source, rowCopyBytes);
    
    source = &lattice[tileHeight][0];
    destination = &lattice[tileHeight * 2][0];
    memcpy(destination, source, rowCopyBytes);

    // Generate vertical tile edges
    std::vector<vec2f32> vertical0(innerYPoints);
    std::vector<vec2f32> vertical1(innerYPoints);
    for (u32 i = 0; i < innerYPoints; ++i)
    {
        vertical0[i] = s_Gradients[s_Permutations[s_Permutations[s_Permutations[0] + i]] & 0xF];
        vertical1[i] = s_Gradients[s_Permutations[s_Permutations[s_Permutations[0] + i + tileHeight]] & 0xF];
    }

    for (u32 verticalTileIndex = 0; verticalTileIndex < 4; ++verticalTileIndex)
    {
        u32 rowIndex = tileHeight * verticalTileIndex + 1;
        for (u32 i = 0; i < innerYPoints; ++i)
        {
            std::vector<vec2f32>& row = lattice[rowIndex];
            row[0] = vertical0[i];
            row[tileWidth] = vertical0[i];
            row[tileWidth * 2] = vertical1[i];
            row[tileWidth * 3] = vertical1[i];
            row[tileWidth * 4] = vertical0[i];

            for (u32 horizontalTileIndex = 0; horizontalTileIndex < 4; ++horizontalTileIndex)
            {
                u32 index = horizontalTileIndex * tileWidth + 1;
                for (u32 j = 0; j < innerXPoints; ++j)
                {
                    row[index] = s_Gradients[s_Permutations[s_Permutations[s_Permutations[index] + rowIndex]] & 0xF];
                    ++index;
                }
            }

            ++rowIndex;
        }
    }

    Generate(data, lattice);
    */

    std::vector<f32> xWeights;
    std::vector<f32> yWeights;

    u32 tileWidth = m_Parameters.latticeWidth >> 2;
    u32 tileHeight = m_Parameters.latticeHeight >> 2;

    u32 maxLatticeY = m_Parameters.latticeHeight + 1;
    u32 maxLatticeX = m_Parameters.latticeWidth + 1;

    TransformCoord transformer;

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
                const vec2f32& tl = s_Gradients[s_Permutations[s_Permutations[s_Permutations[xtl] + ytl]] & 0xF];
                u32 xtr = rightIndex;
                u32 ytr = topIndex;
                transformer(xtr, ytr, tileWidth, tileHeight);
                const vec2f32& tr = s_Gradients[s_Permutations[s_Permutations[s_Permutations[xtr] + ytr]] & 0xF];

                u32 xbl = leftIndex;
                u32 ybl = bottomIndex;
                transformer(xbl, ybl, tileWidth, tileHeight);
                const vec2f32& bl = s_Gradients[s_Permutations[s_Permutations[s_Permutations[xbl] + ybl]] & 0xF];
                u32 xbr = rightIndex;
                u32 ybr = bottomIndex;
                transformer(xbr, ybr, tileWidth, tileHeight);
                const vec2f32& br = s_Gradients[s_Permutations[s_Permutations[s_Permutations[xbr] + ybr]] & 0xF];
                
                f32 x0 = xWeights[xWeightIndex];
                f32 x1 = x0 - 1.0f;
                f32 xWeight = Interpolator()(x0);
                f32 invXWeight = 1.0f - xWeight;

                f32 g0 = tl.x * x0 + tl.y * y0;
                f32 g1 = tr.x * x1 + tr.y * y0;
                f32 g2 = bl.x * x0 + bl.y * y1;
                f32 g3 = br.x * x1 + br.y * y1;

                f32 t = g0 * invXWeight + g1 * xWeight;
                f32 b = g2 * invXWeight + g3 * xWeight;
                f32 value = t * invYWeight + b * yWeight;
                pixels[index] = (value + 1.0f) * 0.5f;

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
void PerlinNoise<Interpolator>::GenerateCorner(ImageData& data) const
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

    const std::vector<vec2f32>& s_Gradients = nPerlin::s_Gradients;
    const std::vector<u32>& s_Permutations = nPerlin::s_Permutations;

    u32 yPoints = m_Parameters.latticeHeight + 1;
    u32 xPoints = m_Parameters.latticeWidth + 1;
    std::vector<std::vector<vec2f32>> lattice(yPoints);
    for (u32 i = 0; i < yPoints; ++i)
        lattice[i].resize(xPoints);

    u32 latticeTileWidth = m_Parameters.latticeWidth >> 2;
    u32 latticeTileHeight = m_Parameters.latticeHeight >> 2;
    u32 xTileUnique = latticeTileWidth - 1;
    u32 yTileUnique = latticeTileHeight - 1;

    vec2f32 cornerR = s_Gradients[s_Permutations[s_Permutations[s_Permutations[0]]] & 0xF];
    vec2f32 cornerG = s_Gradients[s_Permutations[s_Permutations[s_Permutations[1]]] & 0xF];
    vec2f32 cornerValues[2] = {cornerR, cornerG};
    std::vector<vec2f32> horizontalBorders[4];
    std::vector<vec2f32> verticalBorders[4];

    for (u32 i = 0; i < 4; ++i)
    {
        horizontalBorders[i].reserve(xTileUnique);
        verticalBorders[i].reserve(yTileUnique);
        for (u32 j = 0; j < xTileUnique; ++j)
            horizontalBorders[i].push_back(s_Gradients[s_Permutations[s_Permutations[s_Permutations[j + 1]]] & 0xF]);
        for (u32 j = 0; j < yTileUnique; ++j)
            verticalBorders[i].push_back(s_Gradients[s_Permutations[s_Permutations[s_Permutations[0] + j + 1]] & 0xF]);
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

            const std::vector<vec2f32>& topBorder = horizontalBorders[topBorderIndex];
            const std::vector<vec2f32>& bottomBorder = horizontalBorders[bottomBorderIndex];
            const std::vector<vec2f32>& leftBorder = verticalBorders[leftBorderIndex];
            const std::vector<vec2f32>& rightBorder = verticalBorders[rightBorderIndex];

            u32 tileXOffset = latticeTileWidth * tileX;
            u32 tileYOffset = latticeTileHeight * tileY;
            lattice[tileYOffset][tileXOffset] = cornerValues[corners[tl]];
            memcpy(&lattice[tileYOffset][tileXOffset + 1], &topBorder[0], sizeof(vec2f32) * xTileUnique);
            lattice[tileYOffset][tileXOffset + xTileUnique + 1] = cornerValues[corners[tr]];
            ++tileYOffset;
            for (u32 j = 0; j < yTileUnique; ++j)
            {
                u32 rowOffset = tileXOffset;
                std::vector<vec2f32>& row = lattice[tileYOffset];
                row[rowOffset++] = leftBorder[j];
                for (u32 i = 0; i < xTileUnique; ++i)
                    row[rowOffset++] = s_Gradients[s_Permutations[s_Permutations[s_Permutations[1 + i + tileXOffset] + 1 + j + tileYOffset]] & 0xF];
                row[rowOffset] = rightBorder[j];
                ++tileYOffset;
            }
            lattice[tileYOffset][tileXOffset] = cornerValues[corners[bl]];
            memcpy(&lattice[tileYOffset][tileXOffset + 1], &bottomBorder[0], sizeof(vec2f32) * xTileUnique);
            lattice[tileYOffset][tileXOffset + xTileUnique + 1] = cornerValues[corners[br]];
            
            ++tileIndex;
        }
        ++tileIndex;
    }

    Generate(data, lattice);
}
