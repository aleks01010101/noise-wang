#include "TGAFileFormat.hpp"

#include <cmath>
#include <fstream>

void TGAFileFormat::Save(const f32* const pixels, u32 width, u32 height, u32 channels, const std::string& fileName)
{
	std::ofstream file;
	file.open(fileName.c_str(), std::ios::binary);

	Header header;
	header.bitsPerPixel = static_cast<u8>(channels * 8);
	header.colorMapDepth = 0;
	header.colorMapLength = 0;
	header.colorMapOrigin = 0;
	header.colorMapType = 0;
	header.dataType = 2;
	header.height = static_cast<u16>(height);
	header.imageDescriptor = header.bitsPerPixel == 32 ? 40 : 32;
	header.length = 0;
	header.originX = 0;
	header.originY = 0;
	header.width = static_cast<u16>(width);

	file.write(reinterpret_cast<const char*>(&header), sizeof(header));

	u32 pixelCount = width * height;
	char pixel[4];

    const f32* data = pixels;

	if (header.bitsPerPixel == 32)
	{
		for (u32 i = 0; i < pixelCount; ++i)
		{
			pixel[0] = Convert(data[2]);
			pixel[1] = Convert(data[1]);
			pixel[2] = Convert(data[0]);
			pixel[3] = Convert(data[3]);
			file.write(pixel, channels);
			data += channels;
		}
	}
	else
	{
		for (u32 i = 0; i < pixelCount; ++i)
		{
			pixel[0] = Convert(data[2]);
			pixel[1] = Convert(data[1]);
			pixel[2] = Convert(data[0]);
			file.write(pixel, channels);
			data += channels;
		}
	}
}

u8 TGAFileFormat::Convert(f32 value)
{
    f32 clamped = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
    return static_cast<u8>(floor(clamped * 255.0f + 0.5f));
}

f32 TGAFileFormat::Convert(char value)
{
    return static_cast<f32>(static_cast<u8>(value)) / 255.0f;
}

void TGAFileFormat::Load(f32*& pixels, u32& width, u32& height, const std::string& fileName)
{
    std::ifstream file;
	file.open(fileName.c_str(), std::ios::binary);

	Header header;
    file.read(reinterpret_cast<char*>(&header), sizeof(Header));
    width = header.width;
    height = header.height;
    u32 channels = header.bitsPerPixel / 8;

    u32 pixelCount = width * height;
	char pixel[4];
    u32 dataSize = pixelCount * 4;
    pixels = new float[dataSize];

    f32* data = pixels;

    if (channels == 3)
    {
        for (u32 i = 0; i < pixelCount; ++i)
		{
            file.read(pixel, 3);
            data[0] = Convert(pixel[2]);
            data[1] = Convert(pixel[1]);
            data[2] = Convert(pixel[0]);
            data[3] = 1.0f;
            data += 4;
		}
    }
    else
    {
        for (u32 i = 0; i < pixelCount; ++i)
		{
            file.read(pixel, 4);
            data[0] = Convert(pixel[2]);
            data[1] = Convert(pixel[1]);
            data[2] = Convert(pixel[0]);
            data[3] = Convert(pixel[3]);
            data += 4;
		}
    }
}
