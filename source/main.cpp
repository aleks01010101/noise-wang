#include <iostream>

#include "RunTests.hpp"

#include "utility/ArgumentParser.hpp"

void printOptions()
{
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help: print options" << std::endl;
}

i32 main(i32 argc, const char** argv)
{
    if (argc == 1)
    {
        printOptions();
        return 1;
    }

    ArgumentParser arguments;
    arguments.AddKnownArgument("runTests", "t", { "" });
    if (!arguments.Parse(argc, argv))
    {
        printOptions();
        return 1;
    }

    if (arguments.IsEnabled("runTests"))
        return runTests();

    return 0;
}
