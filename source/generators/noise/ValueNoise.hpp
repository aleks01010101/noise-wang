#pragma once

#include <vector>

#include "../../Types.hpp"

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
    
public:
    ValueNoise();
    ~ValueNoise();

public:
    void GenerateNoTiling(ImageData& data) const;
    void GenerateSimple(ImageData& data) const;
    void GenerateWang(ImageData& data) const;
    void GenerateCorner(ImageData& data) const;

    void SetParameters(const Parameters& value) { m_Parameters = value; }

private:
    void Generate(ImageData& data, const std::vector<std::vector<f32>>& lattice) const;

private:
    Parameters m_Parameters;
};