#pragma once

#include "utility/Types.hpp"

class ImageData;

class Waves
{
public:
    struct Parameters
    {
        f32 frequency;
    };

public:
    void GenerateNoTiling(ImageData& data) const;
    void GenerateSimple(ImageData& data) const;
    void GenerateWang(ImageData& data) const;
    void GenerateCorner(ImageData& data) const;

    void SetParameters(const Parameters& value) { parameters = value; }

private:
    void Generate(ImageData& data) const;

private:
    Parameters parameters;
};
