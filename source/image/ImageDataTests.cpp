#include "ImageData.hpp"

#include "testing/TestFixture.hpp"
#include "testing/TestRunner.hpp"
#include "testing/TestSuite.hpp"

// Category 1: Mim dimensions
// 1.1: Square image, pow2, 3 mips
// 1.2: Horizontal rectangle image, pow2, 3 mips
// 1.3: Horizontal line image, pow2, 3 mips
// 1.4: Vertical rectangle image, pow2, 3 mips
// 1.5: Vertical line image, pow2, 3 mips
// Category 2: Mip generation
// 2.1: One channel, two mips
// 2.2: Two channels, two mips
// 2.3: Two channels, three mips

struct ImageDataFixture
{
	virtual ~ImageDataFixture()
	{
		delete image;
	}

	ImageData* image = nullptr;
	u32 mipWidth = 0;
	u32 mipHeight = 0;
};

// Category 1: MIP dimensions
TEST_SUITE(ImageData_MipDimensions)
{
	// 1.1: Square image, pow2, 3 mips
	TEST_FIXTURE(ImageDataFixture, SquarePow2Image3Mips_GetDimensions_ValuesAreCorrect)
	{
		image = new ImageData(4, 4, 1, true);
		const u32 expected[] = {
			4, 2, 1
		};
		const u32 numExpected = sizeof(expected) / sizeof(u32);
		
		for (u32 i = 0; i < numExpected; ++i)
		{
			image->GetDimensions(mipWidth, mipHeight, i);
			Check(mipWidth == expected[i]);
			Check(mipHeight == expected[i]);
		}
	}

	// 1.2: Horizontal rectangle image, pow2, 3 mips
	TEST_FIXTURE(ImageDataFixture, HorizontalRectanglePow2Image3Mips_GetDimensions_ValuesAreCorrect)
	{
		image = new ImageData(4, 2, 1, true);
		const u32 expectedWidth[] = {
			4, 2, 1
		};
		const u32 expectedHeight[] = {
			2, 1, 1
		};
		const u32 numExpected = sizeof(expectedWidth) / sizeof(u32);

		for (u32 i = 0; i < numExpected; ++i)
		{
			image->GetDimensions(mipWidth, mipHeight, i);
			Check(mipWidth == expectedWidth[i]);
			Check(mipHeight == expectedHeight[i]);
		}
	}

	// 1.3: Horizontal line image, pow2, 3 mips
	TEST_FIXTURE(ImageDataFixture, HorizontalLinePow2Image3Mips_GetDimensions_ValuesAreCorrect)
	{
		image = new ImageData(4, 1, 1, true);
		const u32 expectedWidth[] = {
			4, 2, 1
		};
		const u32 expectedHeight[] = {
			1, 1, 1
		};
		const u32 numExpected = sizeof(expectedWidth) / sizeof(u32);

		for (u32 i = 0; i < numExpected; ++i)
		{
			image->GetDimensions(mipWidth, mipHeight, i);
			Check(mipWidth == expectedWidth[i]);
			Check(mipHeight == expectedHeight[i]);
		}
	}

	// 1.4: Vertical rectangle image, pow2, 3 mips
	TEST_FIXTURE(ImageDataFixture, VerticalRectanglePow2Image3Mips_GetDimensions_ValuesAreCorrect)
	{
		image = new ImageData(2, 4, 1, true);
		const u32 expectedWidth[] = {
			2, 1, 1
		};
		const u32 expectedHeight[] = {
			4, 2, 1
		};
		const u32 numExpected = sizeof(expectedWidth) / sizeof(u32);

		for (u32 i = 0; i < numExpected; ++i)
		{
			image->GetDimensions(mipWidth, mipHeight, i);
			Check(mipWidth == expectedWidth[i]);
			Check(mipHeight == expectedHeight[i]);
		}
	}

	// 1.5: Vertical line image, pow2, 3 mips
	TEST_FIXTURE(ImageDataFixture, VerticalLinePow2Image3Mips_GetDimensions_ValuesAreCorrect)
	{
		image = new ImageData(1, 4, 1, true);
		const u32 expectedWidth[] = {
			1, 1, 1
		};
		const u32 expectedHeight[] = {
			4, 2, 1
		};
		const u32 numExpected = sizeof(expectedWidth) / sizeof(u32);

		for (u32 i = 0; i < numExpected; ++i)
		{
			image->GetDimensions(mipWidth, mipHeight, i);
			Check(mipWidth == expectedWidth[i]);
			Check(mipHeight == expectedHeight[i]);
		}
	}
}

// Category 2: Mip generation
TEST_SUITE(ImageData_MipGeneration)
{
	// 2.1: One channel, two mips
	TEST_FIXTURE(ImageDataFixture, OneChannelTwoMips_GenerateMips_ValuesAreCorrect)
	{
		image = new ImageData(2, 2, 1, true);

		f32* pixels = image->GetPixels(0);
		for (u64 i = 0; i < 4; ++i)
			pixels[i] = 0.1f * static_cast<f32>(i);

		image->GenerateMips(0);

		const f32* generated = image->GetPixels(1);
		Check(fabsf(generated[0] - 0.15f) < 0.001f);
	}
	
	// 2.2: Two channels, two mips
	TEST_FIXTURE(ImageDataFixture, TwoChannelsTwoMips_GenerateMips_ValuesAreCorrect)
	{
		image = new ImageData(2, 2, 2, true);

		f32* pixels = image->GetPixels(0);
		for (u64 i = 0; i < 8; ++i)
			pixels[i] = 0.1f * static_cast<f32>(i);

		image->GenerateMips(0);

		const f32* generated = image->GetPixels(1);
		Check(fabsf(generated[0] - 0.3f) < 0.001f);
		Check(fabsf(generated[1] - 0.4f) < 0.001f);
	}
	
	// 2.3: Two channels, three mips
	TEST_FIXTURE(ImageDataFixture, TwoChannelsThreeMips_GenerateMips_ValuesAreCorrect)
	{
		image = new ImageData(4, 4, 2, true);

		f32* pixels = image->GetPixels(0);
		for (u64 i = 0; i < 32; ++i)
			pixels[i] = 0.01f * static_cast<f32>(i);

		image->GenerateMips(0);

		const f32* generated = image->GetPixels(2);
		Check(fabsf(generated[0] - 0.15f) < 0.001f);
		Check(fabsf(generated[1] - 0.16f) < 0.001f);
	}
}
