#pragma once

#include "utility/Types.hpp"

#include <vector>

class ImageData;

template<class Interpolator>
class BetterGradientNoise
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
    typedef std::vector<std::vector<u32>> Lattice;
    static void Generate(const Parameters& parameters, const Lattice& latticeX, const Lattice& latticeY, ImageData& data);
    
    static void EnsureInitialized();
};
