#include "TestRunner.hpp"

#include "TestSuite.hpp"

TestRunner::TestRunner()
{
}

TestRunner::~TestRunner()
{
	for (u64 i = 0, n = suites.size(); i < n; ++i)
		delete suites[i];
}

void TestRunner::Register(TestSuite* suite)
{
	suites.push_back(suite);
}

void TestRunner::Run(TestingContext& context)
{
	u64 n = suites.size();
	for (u64 i = 0; i < n; ++i)
		suites[i]->Prepare(context);
	for (u64 i = 0; i < n; ++i)
		suites[i]->Run(context);
}
