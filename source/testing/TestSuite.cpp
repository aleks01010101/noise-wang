#include "TestSuite.hpp"

#include "Test.hpp"

#include <iostream>

TestSuite::TestSuite(const std::string& suiteName)
	: name(suiteName)
{
}

TestSuite::~TestSuite()
{
	for (u64 i = 0, n = tests.size(); i < n; ++i)
		delete tests[i];
}

void TestSuite::Register(Test* test)
{
	tests.push_back(test);
}

void TestSuite::Prepare(TestingContext& context) const
{
	context.totalCount += tests.size();
}

void TestSuite::Run(TestingContext& context)
{
	for (u64 i = 0, n = tests.size(); i < n; ++i)
	{
		bool failed = !tests[i]->Run();
		++context.runCount;
		if (failed)
		{
			std::cerr << "Test " << tests[i]->GetName() << " failed!" << std::endl;
			++context.failedCount;
		}
	}
}
