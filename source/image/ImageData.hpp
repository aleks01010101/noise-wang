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
    void GetDimensions(u32& width, u32& height, u32 mipLevel) const;
    u32 GetMipLevelCount() const;
    u32 GetChannelCount() const { return m_Channels; }

    f32* GetPixels(u32 mipLevel);
    const f32* GetPixels(u32 mipLevel) const;

    void GenerateMips(u32 base);

private:
    std::vector<std::vector<f32>> m_MipLevels;
    u32 m_Width;
    u32 m_Height;
    u32 m_Channels;
};
