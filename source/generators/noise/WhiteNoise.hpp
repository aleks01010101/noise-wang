#pragma once

#include "generators/TilingMode.hpp"
#include "utility/Types.hpp"

class ImageData;

class WhiteNoise
{
public:
    struct Parameters
    {};

    static void GenerateSimple(const Parameters& parameters, ImageData& data);
    static void GenerateWang(const Parameters& parameters, ImageData& data);
};
