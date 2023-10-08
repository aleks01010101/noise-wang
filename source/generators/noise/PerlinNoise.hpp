#pragma once

#include "utility/Types.hpp"

#include <vector>

class ImageData;

template<class Interpolator>
class PerlinNoise
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
    typedef std::vector<std::vector<f32>> Lattice;
    static void Generate(const Lattice& latticeX, const Lattice& latticeY, const Parameters& parameters, ImageData& data);
    
    static void EnsureInitialized();
};
