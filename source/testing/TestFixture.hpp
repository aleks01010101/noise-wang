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

#define DECLARE_TEST_FIXTURE(Name) \
struct Test_ ## Name \
	: public TestFixture<Fixture> \
{ \
	Test_ ## Name(); \
\
	virtual void DoRun() override; \
};

#define REGISTER_TEST(Name) Register(new Test_ ## Name)

#define IMPLEMENT_TEST_FIXTURE(Suite, Name) \
Suite::Test_ ## Name::Test_ ## Name() \
	: TestFixture(#Name) \
{} \
\
void Suite::Test_ ## Name::DoRun()
