#include "RunTests.hpp"

#include "testing/TestRunner.hpp"

#include <iostream>

i32 runTests()
{
    TestingContext context;

    TestRunner::Instance().Run(context);

    std::cout << "Tests run: " << context.runCount << " out of " << context.totalCount << ", failed: " << context.failedCount << "." << std::endl;

    return -static_cast<int>(context.failedCount);
}
