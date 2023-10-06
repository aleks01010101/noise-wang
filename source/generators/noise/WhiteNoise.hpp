#pragma once

#include "../../Types.hpp"

void GenerateWhiteNoise(f32 x, f32 y, f32 z, f32 w, u32 key, u32* result);
void GenerateWhiteNoise(u32 x, u32 y, u32 z, u32 w, u32 key, u32* result);

class ImageData;

class WhiteNoise
{
public:
    WhiteNoise();
    ~WhiteNoise();

public:
    void GenerateNoTiling(ImageData& data) const;
    void GenerateSimple(ImageData& data) const;
    void GenerateWang(ImageData& data) const;
    void GenerateCorner(ImageData& data) const;
};
