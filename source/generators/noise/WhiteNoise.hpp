#pragma once

#include "generators/TilingMode.hpp"
#include "utility/Types.hpp"

class ImageData;

class WhiteNoise
{
public:
    static void Generate(TilingMode mode, ImageData& data);

private:
    static void GenerateSimple(ImageData& data);
    static void GenerateWang(ImageData& data);
};
