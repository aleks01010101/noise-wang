#pragma once

#include "generators/TilingMode.hpp"

#include "utility/Types.hpp"

class ImageData;

class Checker final
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

    static void Generate(TilingMode mode, const Parameters& parameters, ImageData& data);

private:
    static void GenerateSimple(const Parameters& parameters, ImageData& data);
    static void GenerateWang(const Parameters& parameters, ImageData& data);
    static void Generate(const Parameters& parameters, ImageData& data);
};
