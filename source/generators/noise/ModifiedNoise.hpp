#pragma once

#include <vector>

#include "../../Vector2.hpp"

class ImageData;

template<class Interpolator>
class ModifiedNoise
{
public:
    struct Parameters
    {
        u32 latticeWidth;
        u32 latticeHeight;
    };
    
public:
    ModifiedNoise();
    ~ModifiedNoise();

public:
    void GenerateNoTiling(ImageData& data) const;
    void GenerateSimple(ImageData& data) const;
    void GenerateWang(ImageData& data) const;
    void GenerateCorner(ImageData& data) const;

    void SetParameters(const Parameters& value) { m_Parameters = value; }

private:
    void Generate(ImageData& data, const std::vector<std::vector<vec2f32>>& lattice) const;
    
    static void Initialize();

private:
    Parameters m_Parameters;
};
