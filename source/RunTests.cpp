#include "RunTests.hpp"

#include "testing/TestRunner.hpp"

#include "utility/ArgumentParserTests.hpp"

i32 runTests()
{
    TestingContext context;
    TestRunner runner;

    runner.Register(new ArgumentParserSuite);
    runner.Run(context);

    return -static_cast<int>(context.failedCount);
}
