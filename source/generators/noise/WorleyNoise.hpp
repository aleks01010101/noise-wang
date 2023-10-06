#pragma once

#include "../../Types.hpp"

class ImageData;

class WorleyNoise
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
    
public:
    WorleyNoise();
    ~WorleyNoise();

public:
    void GenerateNoTiling(ImageData& data) const;
    void GenerateSimple(ImageData& data) const;
    void GenerateWang(ImageData& data) const;
    void GenerateCorner(ImageData& data) const;

    void SetParameters(const Parameters& value) { m_Parameters = value; }

private:
    template<class IndexProvider>
    void Generate(const IndexProvider& indexProvider, ImageData& data) const;

private:
    Parameters m_Parameters;
};
