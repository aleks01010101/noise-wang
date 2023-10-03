#pragma once

#include "TestingContext.hpp"

#include <vector>

class Test;
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

	static TestRunner& Instance();

	bool Register(TestSuite* suite);
	bool Register(Test* test);
	void Run(TestingContext& context);

private:
	std::vector<TestSuite*> suites;
};
