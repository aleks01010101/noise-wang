#pragma once

#include "utility/Types.hpp"

#include <vector>

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
    
    static void GenerateSimple(const Parameters& parameters, ImageData& data);
    static void GenerateWang(const Parameters& parameters, ImageData& data);

private:
    template<class IndexProvider>
    static void Generate(const Parameters& parameters, const IndexProvider& indexProvider, ImageData& data);
};