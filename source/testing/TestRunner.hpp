#pragma once

#include "TestingContext.hpp"

#include <vector>

class TestSuite;

class TestRunner final
{
public:
	TestRunner();
	TestRunner(const TestRunner&) = delete;
	TestRunner(TestRunner&&) = delete;
	~TestRunner();

	TestRunner& operator =(const TestRunner&) = delete;
	TestRunner& operator =(TestRunner&&) = delete;

	void Register(TestSuite* suite);
	void Run(TestingContext& context);

private:
	std::vector<TestSuite*> suites;
};
