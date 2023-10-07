#pragma once

#include "generators/TilingMode.hpp"
#include "utility/Types.hpp"

class ImageData;

class WorleyNoise final
{
public:
    struct Parameters
    {
        u32 cellSize;
        u32 cellsPerRow;
        u32 minPointsPerCell;
        u32 maxPointsPerCell;
        u32 cellIndexOffset;
        f32 rAdd;
        f32 gAdd;
        f32 bAdd;
        f32 rMul;
        f32 gMul;
        f32 bMul;
    };

    static void Generate(TilingMode mode, const Parameters& parameters, ImageData& data);

private:
    static void GenerateSimple(const Parameters& parameters, ImageData& data);
    static void GenerateWang(const Parameters& parameters, ImageData& data);

    template<class IndexProvider>
    static void Generate(const IndexProvider& indexProvider, const Parameters& parameters, ImageData& data);
};
