#pragma once

#include <vector>

#include "../../Types.hpp"

class ImageData;

class GaborNoise
{
public:
    struct Parameters
    {
        f32 cellSize;
        u32 numberOfImpulsesPerCell;
        u32 numberOfImpulsesPerCellCap;
        u32 cellOffset;
        f32 gaussianMagnitude;
        f32 gaussianWidth;
        f32 frequencyMagnitudeMin;
        f32 frequencyMagnitudeMax;
        f32 frequencyOrientationMin;
        f32 frequencyOrientationMax;
    };
    
public:
    GaborNoise();
    ~GaborNoise();

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