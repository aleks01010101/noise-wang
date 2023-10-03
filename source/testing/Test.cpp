#include "Test.hpp"

Test::Test(const std::string& testName)
	: name(testName)
{
}

bool Test::Run()
{
	DoRun();
	return !failed;
}

void Test::Check(bool value)
{
	if (!value)
		failed = true;
}
