#pragma once

class ImageData;

struct ChannelConverter final
{
	static void RToRRR1(const ImageData& source, ImageData& destination);
	static void RGToRG01(const ImageData& source, ImageData& destination);

	static void Copy(const ImageData& source, ImageData& destination);
};
