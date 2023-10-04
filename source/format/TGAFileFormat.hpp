#pragma once

#include <string>

#include "utility/Types.hpp"

class TGAFileFormat
{
private:
    __pragma (pack(push, 1))
	struct Header
	{
		u8 length;
		u8 colorMapType;
		u8 dataType;
		u16 colorMapOrigin;
		u16 colorMapLength;
		u8 colorMapDepth;
		u16 originX;
		u16 originY;
		u16 width;
		u16 height;
		u8 bitsPerPixel;
		u8 imageDescriptor;
	};
	__pragma (pack(pop))

public:
	static void Save(const f32* const pixels, u32 width, u32 height, u32 channels, const std::string& fileName);
    static void Load(f32*& pixels, u32& width, u32& height, const std::string& fileName);

private:
    static u8 Convert(f32 value);
    static f32 Convert(char value);
};