#include "ChannelConversion.hpp"

#include "ImageData.hpp"

#include "testing/TestFixture.hpp"
#include "testing/TestRunner.hpp"
#include "testing/TestSuite.hpp"

// Category 1: Channel conversion
// 1.1: R expanded to RRR1 with mips
// 1.2: RG expanded to RG01 with mips
// Category 2: Copying
// 2.1: R copy with mips
// 2.2: RG copy with mips
// 2.3: RGB copy with mips
// 2.4: RGBA copy with mips

struct ChannelConverterFixture
{
	virtual ~ChannelConverterFixture()
	{
		delete source;
		delete destination;
	}

	bool CheckPixel(const f32* destination, f32 r, f32 epsilon)
	{
		return fabsf(destination[0] - r) < epsilon;
	}

	bool CheckPixel(const f32* destination, f32 r, f32 g, f32 b, f32 a, f32 epsilon)
	{
		return fabsf(destination[0] - r) < epsilon
			&& fabsf(destination[1] - g) < epsilon
			&& fabsf(destination[2] - b) < epsilon
			&& fabsf(destination[3] - a) < epsilon;
	}

	void FillSource()
	{
		u32 w;
		u32 h;
		source->GetDimensions(w, h, 0);
		u32 numChannels = source->GetChannelCount();

		f32* values = source->GetPixels(0);
		
		u32 numValues = w * h * numChannels;
		for (u32 i = 0; i < numValues; ++i)
			values[i] = static_cast<f32>(i + 1);

		source->GenerateMips(0);
	}

	bool AreAllMipsEqual()
	{
		u32 numChannels = source->GetChannelCount();
		for (u32 mip = 0, numMips = source->GetMipLevelCount(); mip < numMips; ++mip)
		{
			u32 w;
			u32 h;
			source->GetDimensions(w, h, mip);

			u32 numValues = w * h * numChannels;

			const f32* sourceData = source->GetPixels(mip);
			const f32* destinationData = destination->GetPixels(mip);

			for (u32 i = 0; i < numValues; ++i)
			{
				if (fabsf(sourceData[i] - destinationData[i]) >= 0.001f)
					return false;
			}
		}

		return true;
	}

	ImageData* source = nullptr;
	ImageData* destination = nullptr;
};

// Category 1: Channel conversion
TEST_SUITE(ChannelConverter_ChannelConversion)
{
	// 1.1: R expanded to RRR1 with mips
	TEST_FIXTURE(ChannelConverterFixture, SingleChannelImage_ConvertedToFourChannelImageWithoutAlpha_ResultIsCorrect)
	{
		source = new ImageData(2, 2, 1, true);
		destination = new ImageData(2, 2, 4, true);

		f32* sourcePixels = source->GetPixels(0);
		for (u32 i = 0; i < 4; ++i)
			sourcePixels[i] = static_cast<f32>(i + 1);
		source->GenerateMips(0);

		ChannelConverter::RToRRR1(*source, *destination);

		const f32* destinationPixels = destination->GetPixels(0);
		for (u32 i = 0; i < 4; ++i)
		{
			f32 expected = static_cast<f32>(i + 1);
			Check(CheckPixel(destinationPixels, expected, expected, expected, 1.0f, 0.001f));
			destinationPixels += 4;
		}
		destinationPixels = destination->GetPixels(1);
		f32 expected = 2.5f;
		Check(CheckPixel(destinationPixels, expected, expected, expected, 1.0f, 0.001f));
	}
	
	// 1.2: RG expanded to RG01 with mips
	TEST_FIXTURE(ChannelConverterFixture, TwoChannelImage_ConvertedToFourChannelImageWithoutBlueAndAlpha_ResultIsCorrect)
	{
		source = new ImageData(2, 2, 2, true);
		destination = new ImageData(2, 2, 4, true);

		f32* sourcePixels = source->GetPixels(0);
		for (u32 i = 0; i < 8; ++i)
			sourcePixels[i] = static_cast<f32>(i + 1);
		source->GenerateMips(0);

		ChannelConverter::RGToRG01(*source, *destination);

		const f32* destinationPixels = destination->GetPixels(0);
		for (u32 i = 0; i < 4; ++i)
		{
			f32 expectedR = static_cast<f32>(i * 2 + 1);
			f32 expectedG = expectedR + 1.0f;
			Check(CheckPixel(destinationPixels, expectedR, expectedG, 0.0f, 1.0f, 0.001f));
			destinationPixels += 4;
		}
		destinationPixels = destination->GetPixels(1);
		f32 expectedR = 4.0f;
		f32 expectedG = 5.0f;
		Check(CheckPixel(destinationPixels, expectedR, expectedG, 0.0f, 1.0f, 0.001f));
	}
}

// Category 2: Copying
TEST_SUITE(ChannelConverter_Copying)
{
	// 2.1: R copy with mips
	TEST_FIXTURE(ChannelConverterFixture, SingleChannelImage_Copied_ResultIsCorrect)
	{
		source = new ImageData(2, 2, 1, true);
		destination = new ImageData(2, 2, 1, true);

		FillSource();
		ChannelConverter::Copy(*source, *destination);

		Check(AreAllMipsEqual());
	}
	
	// 2.2: RG copy with mips
	TEST_FIXTURE(ChannelConverterFixture, TwoChannelImage_Copied_ResultIsCorrect)
	{
		source = new ImageData(2, 2, 2, true);
		destination = new ImageData(2, 2, 2, true);

		FillSource();
		ChannelConverter::Copy(*source, *destination);

		Check(AreAllMipsEqual());
	}
	
	// 2.3: RGB copy with mips
	TEST_FIXTURE(ChannelConverterFixture, ThreeChannelImage_Copied_ResultIsCorrect)
	{
		source = new ImageData(2, 2, 3, true);
		destination = new ImageData(2, 2, 3, true);

		FillSource();
		ChannelConverter::Copy(*source, *destination);

		Check(AreAllMipsEqual());
	}
	
	// 2.4: RGBA copy with mips
	TEST_FIXTURE(ChannelConverterFixture, FourChannelImage_Copied_ResultIsCorrect)
	{
		source = new ImageData(2, 2, 4, true);
		destination = new ImageData(2, 2, 4, true);

		FillSource();
		ChannelConverter::Copy(*source, *destination);

		Check(AreAllMipsEqual());
	}
}
