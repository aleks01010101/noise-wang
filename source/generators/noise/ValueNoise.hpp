#pragma once

#include "generators/TilingMode.hpp"
#include "utility/Types.hpp"

#include <vector>

class ImageData;

template<class Interpolator>
class ValueNoise
{
public:
    struct Parameters
    {
        u32 latticeWidth;
        u32 latticeHeight;
        f32 rangeMin;
        f32 rangeMax;
    };

    static void Generate(TilingMode mode, const Parameters& parameters, ImageData& data);

private:
    static void GenerateSimple(const Parameters& parameters, ImageData& data);
    static void GenerateWang(const Parameters& parameters, ImageData& data);
    static void Generate(const std::vector<std::vector<f32>>& lattice, const Parameters& parameters, ImageData& data);
};