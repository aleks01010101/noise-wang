#include <iostream>

#include "RunTests.hpp"

#include "generators/noise/WhiteNoise.hpp"
#include "generators/noise/WorleyNoise.hpp"
#include "generators/simple/Checker.hpp"
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

// Worley defaults
static constexpr u64 kDefaultMinPointsPerCell = 2;
static constexpr u64 kDefaultMaxPointsPerCell = 2;
static constexpr u64 kDefaultCellSize = 32;

// Image defaults
static constexpr u64 kDefaultWidth = 1024;
static constexpr u64 kDefaultHeight = 1024;

static void printOptions(const ArgumentParser& parser)
{
    std::cout << "Options:" << std::endl;
    parser.PrintOptions();
}

static ImageData* createImage(const ArgumentParser& parser, u32 numChannels)
{
    u32 w = parser.GetValueAs<u32>("width");
    u32 h = parser.GetValueAs<u32>("height");

    if (w == 0 || h == 0)
        return nullptr;

    return new ImageData(w, h, numChannels, parser.IsEnabled("mipmaps"));
}

static void generateChecker(TilingMode mode, const ArgumentParser& parser, ImageData& result)
{
    Checker::Parameters parameters;
    parameters.brightMax = parser.GetValueAs<f32>("checker-bright-max") / 255.0f;
    parameters.brightMin = parser.GetValueAs<f32>("checker-bright-min") / 255.0f;
    parameters.darkMax = parser.GetValueAs<f32>("checker-dark-max") / 255.0f;
    parameters.darkMin = parser.GetValueAs<f32>("checker-dark-min") / 255.0f;
    parameters.tileHeight = parser.GetValueAs<u32>("checker-tile-height");
    parameters.tileWidth = parser.GetValueAs<u32>("checker-tile-width");
    
    Checker::Generate(mode, parameters, result);
}

static void generateWorley(TilingMode mode, const ArgumentParser& parser, ImageData& result)
{
    WorleyNoise::Parameters parameters;

    parameters.minPointsPerCell = parser.GetValueAs<u32>("worley-min-points-per-cell");
    parameters.maxPointsPerCell = parser.GetValueAs<u32>("worley-max-points-per-cell");
    parameters.cellSize = parser.GetValueAs<u32>("worley-cell-size");
    parameters.cellsPerRow = result.GetWidth() / parameters.cellSize;
    parameters.cellIndexOffset = 1;
    parameters.rMul = 1.0f;
    parameters.gMul = 1.0f;
    parameters.bMul = 1.0f;
    parameters.rAdd = 0.0f;
    parameters.gAdd = 0.0f;
    parameters.bAdd = 0.0f;

    WorleyNoise::Generate(mode, parameters, result);
}

static void generateWhiteNoise(TilingMode mode, ImageData& result)
{
    WhiteNoise::Generate(mode, result);
}

i32 main(i32 argc, const char** argv)
{
    ArgumentParser arguments;
    // Generic parameters
    arguments.AddKnownArgument("run-tests", "rt", { "" }, {"run unit tests"});
    arguments.AddKnownArgument("help", "h", { "" }, { "print options" });

    arguments.AddKnownArgument("generator", "g", { "checker", "worley", "white" }, {
        "select image generation algorithm",

        "generate checker pattern with random tile brightness",
        "generate cellular pattern using Worley noise",
        "generate white noise",
        });
    arguments.AddKnownArgument("tiling", "t", { "simple", "wang" }, {
        "select image tiling algorithm",

        "simple repetition tiling",
        "wang tiles",
        });

    // Checker parameters
    arguments.AddKnownArgument("cheker-bright-max", "bmax", {}, { "maximum brightness of a bright checker tile. Must be in range [0; 255]" }, kDefaultBrightMax);
    arguments.AddKnownArgument("checker-bright-min", "bmin", {}, { "minimum brightness of a bright checker tile. Must be in range [0; 255]" }, kDefaultBrightMin);
    arguments.AddKnownArgument("checker-dark-max", "dmax", {}, { "maximum brightness of a dark checker tile. Must be in range [0; 255]" }, kDefaultDarkMax);
    arguments.AddKnownArgument("checker-dark-min", "dmin", {}, { "minimum brightness of a dark checker tile. Must be in range [0; 255]" }, kDefaultDarkMin);
    arguments.AddKnownArgument("checker-tile-width", "tw", {}, { "width of a checker tile. Must be a divisor of image width" }, kDefaultTileWidth);
    arguments.AddKnownArgument("checker-tile-height", "th", {}, { "height of a checker tile. Must be a divisor of image height" }, kDefaultTileHeight);

    // Worley noise parameters
    arguments.AddKnownArgument("worley-min-points-per-cell", "minppc", {}, { "minimum number of points per cell for Worley noise" }, kDefaultMinPointsPerCell);
    arguments.AddKnownArgument("worley-max-points-per-cell", "maxppc", {}, { "maximum number of points per cell for Worley noise" }, kDefaultMaxPointsPerCell);
    arguments.AddKnownArgument("worley-cell-size", "cs", {}, { "size of a single cell for Worley noise" }, kDefaultCellSize);

    // Image parameters
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

    enum class Generator
    {
        kChecker,
        kWorley,
        kWhite,
    };

    u32 numChannels = 1;

    Generator selected = arguments.GetValueAs<Generator>("generator");
    TilingMode tiling = arguments.GetValueAs<TilingMode>("tiling");

    if (selected == Generator::kWorley)
        numChannels = 4;

    ImageData* generated = createImage(arguments, numChannels);
    if (generated == nullptr)
    {
        std::cout << "Incorrect image parameters provided." << std::endl;
        printOptions(arguments);
        return 2;
    }

    switch (selected)
    {
    case Generator::kChecker:
        generateChecker(tiling, arguments, *generated);
        break;
    case Generator::kWorley:
        generateWorley(tiling, arguments, *generated);
        break;
    case Generator::kWhite:
        generateWhiteNoise(tiling, *generated);
        break;
    };

    if (numChannels == 1)
    {
        ImageData* expanded = new ImageData(generated->GetWidth(), generated->GetHeight(), 4, generated->GetMipLevelCount() > 1);
        ChannelConverter::RToRRR1(*generated, *expanded);
        expanded->Save("output");

        delete expanded;
    }
    else
    {
        generated->Save("output");
    }
    delete generated;

    return 0;
}
