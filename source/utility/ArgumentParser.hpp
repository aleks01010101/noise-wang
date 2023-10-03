#pragma once

#include "Types.hpp"

#include <string>
#include <vector>

class ArgumentParser final
{
public:
	typedef std::vector<std::string> ValidValues;

	ArgumentParser() = default;
	ArgumentParser(const ArgumentParser&) = delete;
	ArgumentParser(ArgumentParser&&) = delete;
	~ArgumentParser() = default;

	ArgumentParser& operator =(const ArgumentParser&) = delete;
	ArgumentParser& operator =(ArgumentParser&&) = delete;

	bool IsEnabled(const std::string& name) const { return GetValue(name) == 1; }
	u64 GetValue(const std::string& name) const;
	template<typename T>
	T GetValueAs(const std::string& name) const { return static_cast<T>(GetValue(name)); }

	// No checking for duplicates is performed. First added wins.
	// If validValues is empty, u64 is assumed; otherwise the result of parsing is the index
	// of the value in the array. If the only valid value is an empty string, there is no parameter.
	void AddKnownArgument(const std::string& name, const std::string& shortName, const ValidValues& knownValues);
	void AddKnownArgument(const std::string& name, const std::string& shortName, const ValidValues& knownValues,
		u64 defaultValue);

	// No checking for duplicates is performed. Last parameter wins.
	// Each parameter can have 0 of 1 arguments that are parsed based on known arguments.
	bool Parse(i32 argc, const char** argv);

private:
	typedef std::vector<std::string> ArgumentNames;
	typedef std::vector<u64> ArgumentValues;

	u64 Find(const char* argument) const;
	u64 Find(const char* argument, const ArgumentNames& names) const;
	bool TryParseName(const char* argument, u64& lastArgumentID);
	bool TryParseValue(const char* argument, u64& lastArgumentID);

	ArgumentNames argumentNames;
	ArgumentNames shortArgumentNames;
	ArgumentValues argumentValues;
	std::vector<ValidValues> validValues;
};
