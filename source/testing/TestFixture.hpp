#pragma once

#include "Test.hpp"

template<class T>
struct TestFixture
	: public Test
	, public T
{
	TestFixture() = delete;
	explicit TestFixture(const std::string& name)
		: Test(name)
	{}
};

#define TEST_FIXTURE(FixtureName, Name) \
struct Test_ ## Name \
	: public TestFixture<FixtureName> \
{ \
	Test_ ## Name() \
		: TestFixture(#Name)\
	{\
	}\
\
	virtual void DoRun() override; \
}; \
\
static const bool kTest_ ## Name ## _Registered = TestRunner::Instance().Register(new Test_ ## Name); \
\
void Test_ ## Name::DoRun()
