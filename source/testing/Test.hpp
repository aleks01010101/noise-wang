#pragma once

#include <string>

class Test
{
public:
	Test() = delete;
	explicit Test(const std::string& testName);
	Test(const Test&) = delete;
	Test(Test&&) = delete;
	virtual ~Test() = default;

	Test& operator =(const Test&) = delete;
	Test& operator =(Test&&) = delete;

	bool Run();
	const std::string& GetName() const { return name; }

protected:
	void Check(bool value);

	virtual void DoRun() = 0;

private:
	std::string name;
	bool failed = false;
};
