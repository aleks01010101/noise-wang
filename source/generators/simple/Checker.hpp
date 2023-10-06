#pragma once

#include "utility/Types.hpp"

class ImageData;

class Checker
{
public:
    struct Parameters
    {
        u32 tileWidth;
        u32 tileHeight;
        f32 darkMin;
        f32 darkMax;
        f32 brightMin;
        f32 brightMax;
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
