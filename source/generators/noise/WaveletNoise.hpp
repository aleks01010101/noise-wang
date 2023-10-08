#pragma once

#include "generators/TilingMode.hpp"
#include "utility/Types.hpp"

class ImageData;

template<class Interpolator>
class WaveletNoise
{
public:
    struct Parameters
    {
        u32 latticeWidth;
        u32 latticeHeight;
    };
    
    static void GenerateSimple(const Parameters& parameters, ImageData& data);
    static void GenerateWang(const Parameters& parameters, ImageData& data);

private:
    static void Generate(const Parameters& parameters, ImageData& data, ImageData& baseNoise);
};
