#include <iostream>

#include "RunTests.hpp"

#include "generators/simple/Checker.hpp"
#include "generators/simple/Waves.hpp"
#include "image/ChannelConversion.hpp"
#include "image/ImageData.hpp"
#include "utility/ArgumentParser.hpp"

// Checker defaults
static constexpr u64 kDefaultBrightMax = 255;
static constexpr u64 kDefaultBrightMin = 192;
static constexpr u64 kDefaultDarkMax = 64;
static constexpr u64 kDefaultDarkMin = 0;
static constexpr u64 kDefaultTileWidth = 64;
static constexpr u64 kDefaultTileHeight = 64;

// Wave defaults
static constexpr u64 kDefaultFrequency = 32;

// Image defaults
static constexpr u64 kDefaultWidth = 1024;
static constexpr u64 kDefaultHeight = 1024;

void printOptions(const ArgumentParser& parser)
{
    std::cout << "Options:" << std::endl;
    parser.PrintOptions();
}

ImageData* createImage(const ArgumentParser& parser)
{
    u32 w = parser.GetValueAs<u32>("width");
    u32 h = parser.GetValueAs<u32>("height");

    if (w == 0 || h == 0)
        return nullptr;

    return new ImageData(w, h, 1, parser.IsEnabled("mipmaps"));
}

void generateChecker(const ArgumentParser& parser, ImageData& result)
{
    Checker::Parameters parameters;
    parameters.brightMax = parser.GetValueAs<f32>("bright-max") / 255.0f;
    parameters.brightMin = parser.GetValueAs<f32>("bright-min") / 255.0f;
    parameters.darkMax = parser.GetValueAs<f32>("dark-max") / 255.0f;
    parameters.darkMin = parser.GetValueAs<f32>("dark-min") / 255.0f;
    parameters.tileHeight = parser.GetValueAs<u32>("tile-height");
    parameters.tileWidth = parser.GetValueAs<u32>("tile-width");
    Checker generator;
    generator.SetParameters(parameters);
    generator.GenerateSimple(result);
}

void generateWaves(const ArgumentParser& parser, ImageData& result)
{
    Waves::Parameters parameters;
    parameters.frequency = parser.GetValueAs<f32>("frequency");
    Waves generator;
    generator.SetParameters(parameters);
    generator.GenerateSimple(result);
}

i32 main(i32 argc, const char** argv)
{
    ArgumentParser arguments;
    arguments.AddKnownArgument("run-tests", "rt", { "" }, {"run unit tests"});
    arguments.AddKnownArgument("help", "h", { "" }, { "print options" });

    arguments.AddKnownArgument("type", "t", { "checker", "waves" }, {"select image generation algorithm",
        "generate checker pattern with random tile brightness",
        "generate horizontal lines oscillating between black and white",
        });

    // Checker parameters
    arguments.AddKnownArgument("bright-max", "bmax", {}, { "maximum brightness of a bright checker tile. Must be in range [0; 255]" }, kDefaultBrightMax);
    arguments.AddKnownArgument("bright-min", "bmin", {}, { "minimum brightness of a bright checker tile. Must be in range [0; 255]" }, kDefaultBrightMin);
    arguments.AddKnownArgument("dark-max", "dmax", {}, { "maximum brightness of a dark checker tile. Must be in range [0; 255]" }, kDefaultDarkMax);
    arguments.AddKnownArgument("dark-min", "dmin", {}, { "minimum brightness of a dark checker tile. Must be in range [0; 255]" }, kDefaultDarkMin);
    arguments.AddKnownArgument("tile-width", "tw", {}, { "width of a checker tile. Must be a divisor of image width" }, kDefaultTileWidth);
    arguments.AddKnownArgument("tile-height", "th", {}, { "height of a checker tile. Must be a divisor of image height" }, kDefaultTileHeight);

    // Wave parameters
    arguments.AddKnownArgument("frequency", "f", {}, { "wave frequency in pixels. Must be greater than 2" }, kDefaultFrequency);

    arguments.AddKnownArgument("width", "w", {}, { "image width. Must be greater than 0" }, kDefaultWidth);
    arguments.AddKnownArgument("height", "h", {}, { "image height. Must be greater than 0" }, kDefaultHeight);
    arguments.AddKnownArgument("mipmaps", "m", { "" }, { "generate mipmaps" });

    if (!arguments.Parse(argc, argv))
    {
        printOptions(arguments);
        return 1;
    }

    if (arguments.IsEnabled("run-tests"))
        return runTests();
    if (arguments.IsEnabled("help"))
    {
        printOptions(arguments);
        return 1;
    }

    ImageData* generated = createImage(arguments);
    if (generated == nullptr)
    {
        std::cout << "Incorrect image parameters provided." << std::endl;
        printOptions(arguments);
        return 2;
    }

    enum class Generator
    {
        kChecker,
        kWaves,
    };

    Generator selected = arguments.GetValueAs<Generator>("type");
    switch (selected)
    {
    case Generator::kWaves:
        generateWaves(arguments, *generated);
        break;
    case Generator::kChecker:
        generateChecker(arguments, *generated);
        break;
    };

    ImageData* expanded = new ImageData(generated->GetWidth(), generated->GetHeight(), 4, generated->GetMipLevelCount() > 1);
    ChannelConverter::RToRRR1(*generated, *expanded);
    expanded->Save("output");

    delete expanded;
    delete generated;

    return 0;
}
