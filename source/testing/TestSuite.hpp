#pragma once

#include "TestingContext.hpp"

#include <string>
#include <vector>

class Test;

class TestSuite
{
public:
	TestSuite() = delete;
	explicit TestSuite(const std::string& suiteName);
	TestSuite(const TestSuite&) = delete;
	TestSuite(TestSuite&&) = delete;
	virtual ~TestSuite();

	TestSuite& operator =(const TestSuite&) = delete;
	TestSuite& operator =(TestSuite&&) = delete;

	void Register(Test* test);

	void Prepare(TestingContext& context) const;
	void Run(TestingContext& context);

private:
	std::vector<Test*> tests;
	std::string name;
};

#define TEST_SUITE(Name)\
class Suite_ ## Name \
	: public TestSuite \
{ \
public: \
	Suite_ ## Name() \
		: TestSuite(#Name) \
	{ \
	} \
}; \
 \
static const bool kSuite_ ## Name ## _Registered = TestRunner::Instance().Register(new Suite_ ## Name); \
\
namespace Suite_ ## Name ## _Tests
