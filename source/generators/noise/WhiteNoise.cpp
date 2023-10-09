#include "WhiteNoise.hpp"

#include "image/ImageData.hpp"

#include <vector>

static std::vector<u32> S;
static std::vector<u32> K;

void Initialize()
{
    if (S.size() != 0)
        return;

    S.resize(64);
    for (u32 i = 0; i < 4; ++i)
    {
        S[i * 4] = 7;
        S[i * 4 + 1] = 12;
        S[i * 4 + 2] = 17;
        S[i * 4 + 3] = 22;

        S[i * 4 + 16] = 5;
        S[i * 4 + 17] = 9;
        S[i * 4 + 18] = 14;
        S[i * 4 + 19] = 20;

        S[i * 4 + 32] = 4;
        S[i * 4 + 33] = 11;
        S[i * 4 + 34] = 16;
        S[i * 4 + 35] = 23;

        S[i * 4 + 48] = 6;
        S[i * 4 + 49] = 10;
        S[i * 4 + 50] = 15;
        S[i * 4 + 51] = 21;
    }

    K.resize(64);
    double d = static_cast<double>(0x100000000ull);
    for (u32 i = 0; i < 64; ++i)
    {
        K[i] = static_cast<u32>(floor(d * abs(sin(i + 1))));
    }
}

static void GenerateWhiteNoise(u32 x, u32 y, u32 z, u32 w, u32 key, u32* result)
{
    Initialize();

    u32 data[16] = {
        x ^ key, y ^ key, z ^ key, w ^ key,
        0x80000000, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 128
    };
    
    u32 A = 0x67452301;
    u32 B = 0xefcdab89;
    u32 C = 0x98badcfe;
    u32 D = 0x10325476;

    u32 F;
    u32 g;
    u32 s;

    for (u32 i = 0; i < 16; ++i)
    {
        F = ((B & C) | (~B & D)) + A + K[i] + data[i];
        A = D;
        D = C;
        C = B;
        s = S[i];
        B += (F << s) | (F >> (32 - s));
    }
    for (u32 i = 16; i < 32; ++i)
    {
        g = (i * 5 + 1) & 0xF;

        F = ((D & B) | (~D & C)) + A + K[i] + data[g];
        A = D;
        D = C;
        C = B;
        s = S[i];
        B += (F << s) | (F >> (32 - s));
    }
    for (u32 i = 32; i < 48; ++i)
    {
        g = (i * 3 + 5) & 0xF;

        F = (B ^ C ^ D) + A + K[i] + data[g];
        A = D;
        D = C;
        C = B;
        s = S[i];
        B += (F << s) | (F >> (32 - s));
    }
    for (u32 i = 48; i < 64; ++i)
    {
        g = (i * 7) & 0xF;

        F = (C ^ (B | ~D)) + A + K[i] + data[g];
        A = D;
        D = C;
        C = B;
        s = S[i];
        B += (F << s) | (F >> (32 - s));
    }

    result[0] = A;
    result[1] = B;
    result[2] = C;
    result[3] = D;
}

static f32 toFloat(u32 value)
{
    return static_cast<f32>(static_cast<f64>(value) / static_cast<f64>(UINT32_MAX));
}

void WhiteNoise::GenerateSimple(const Parameters&, ImageData& data)
{
    const u32 mips = data.GetMipLevelCount();
    for (u32 mip = 0; mip < mips; ++mip)
    {
        u32 w;
        u32 h;
        data.GetDimensions(w, h, mip);

        f32* pixels = data.GetPixels(mip);

        u32 buffer[4];
        u32 index = 0;
        for (u32 y = 0; y < h; ++y)
        {
            for (u32 x = 0; x < w; ++x)
            {
                GenerateWhiteNoise(x, y, 0, 0, 0, buffer);
                pixels[index] = toFloat(buffer[0]);
                ++index;
            }
        }
    }
}

void WhiteNoise::GenerateWang(const Parameters& parameters, ImageData& data)
{
    GenerateSimple(parameters, data);
}
