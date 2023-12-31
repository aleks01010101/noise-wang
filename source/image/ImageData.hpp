#pragma once

#include "utility/Types.hpp"

#include <vector>
#include <string>

class ImageData
{
public:
    ImageData() = delete;
    ImageData(const ImageData&) = delete;
    ImageData(ImageData&&) = delete;
    ImageData(u32 width, u32 height, u32 channels, bool generateMipChain);
    ~ImageData() = default;

    ImageData& operator =(const ImageData&) = delete;
    ImageData& operator =(ImageData&&) = delete;

    void Save(const std::string& baseFileName) const;
    void GetDimensions(u32& outWidth, u32& outHeight, u32 mipLevel) const;
    u32 GetWidth() const { return width; }
    u32 GetHeight() const { return height; }
    u32 GetMipLevelCount() const;
    u32 GetChannelCount() const { return channels; }

    f32* GetPixels(u32 mipLevel);
    const f32* GetPixels(u32 mipLevel) const;

    void GenerateMips(u32 base);

private:
    std::vector<std::vector<f32>> mipLevels;
    u32 width;
    u32 height;
    u32 channels;
};
