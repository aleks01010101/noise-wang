#include <iostream>

#include "RunTests.hpp"

#include "utility/ArgumentParser.hpp"

void printOptions()
{
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help: print options" << std::endl;
    std::cout << "  -rt, --run-tests: run unit tests" << std::endl;
}

i32 main(i32 argc, const char** argv)
{
    ArgumentParser arguments;
    arguments.AddKnownArgument("run-tests", "rt", { "" });
    arguments.AddKnownArgument("help", "h", { "" });
    if (!arguments.Parse(argc, argv))
    {
        printOptions();
        return 1;
    }

    if (arguments.IsEnabled("run-tests"))
        return runTests();
    if (arguments.IsEnabled("help") || argc == 1)
    {
        printOptions();
        return 1;
    }

    return 0;
}
