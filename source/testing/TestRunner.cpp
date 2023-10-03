#include "TestRunner.hpp"

#include "Test.hpp"
#include "TestSuite.hpp"

#include <cassert>

TestRunner::TestRunner()
{
}

TestRunner::~TestRunner()
{
	for (u64 i = 0, n = suites.size(); i < n; ++i)
		delete suites[i];
}

TestRunner& TestRunner::Instance()
{
	static TestRunner testRunner;
	return testRunner;
}

bool TestRunner::Register(TestSuite* suite)
{
	suites.push_back(suite);
	return true;
}

bool TestRunner::Register(Test* test)
{
	assert(!suites.empty());
	suites.back()->Register(test);
	return true;
}

void TestRunner::Run(TestingContext& context)
{
	u64 n = suites.size();
	for (u64 i = 0; i < n; ++i)
		suites[i]->Prepare(context);
	for (u64 i = 0; i < n; ++i)
		suites[i]->Run(context);
}
