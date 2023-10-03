#include "ArgumentParser.hpp"

#include <cassert>
#include <iostream>

static const u64 kInvalidID = ~0ull;

u64 ArgumentParser::GetValue(const std::string& name) const
{
	auto begin = argumentNames.begin();
	auto end = argumentNames.end();
	auto it = std::find(begin, end, name);
	assert(it != end);
	return argumentValues[it - begin];
}

void ArgumentParser::AddKnownArgument(const std::string& name, const std::string& shortName, const ValidValues& knownValues)
{
	AddKnownArgument(name, shortName, knownValues, 0);
}

void ArgumentParser::AddKnownArgument(const std::string& name, const std::string& shortName, const ValidValues& knownValues, u64 defaultValue)
{
	argumentNames.push_back(name);
	shortArgumentNames.push_back(shortName);
	validValues.push_back(knownValues);
	argumentValues.push_back(defaultValue);
}

bool ArgumentParser::Parse(i32 argc, const char** argv)
{
	if (argc <= 1)
		return true;

	u64 numArguments = static_cast<u64>(argc);
	u64 lastArgumentID = kInvalidID;

	// Skip executable name (idx 0)
	for (u64 i = 1; i < numArguments; ++i)
	{
		const char* currentArgument = argv[i];

		if (lastArgumentID == kInvalidID)
		{
			if (!TryParseName(currentArgument, lastArgumentID))
				return false;
		}
		else
		{
			if (!TryParseValue(currentArgument, lastArgumentID))
				return false;
		}
	}

	return true;
}

u64 ArgumentParser::Find(const char* argument) const
{
	// Skip leading '-'
	++argument;
	if (argument[0] == '-')
	{
		// Skip leading '-'
		++argument;
		return Find(argument, argumentNames);
	}
	else
		return Find(argument, shortArgumentNames);
}

u64 ArgumentParser::Find(const char* argument, const ArgumentNames& names) const
{
	auto begin = names.begin();
	auto end = names.end();
	std::string name(argument);
	auto it = std::find(begin, end, name);
	return it != end ? it - begin : kInvalidID;
}

bool ArgumentParser::TryParseName(const char* argument, u64& lastArgumentID)
{
	if (argument[0] != '-')
	{
		std::cerr << "Expected parameter name, but got '" << argument << "'." << std::endl;
		return false;
	}
	lastArgumentID = Find(argument);

	if (lastArgumentID == kInvalidID)
	{
		std::cerr << "Parameter '" << argument << "' not found." << std::endl;
		return false;
	}

	const ValidValues& allValid = validValues[lastArgumentID];
	if (allValid.size() == 1 && allValid[0].empty())
	{
		// Enable the argument without parameters.
		argumentValues[lastArgumentID] = 1;
		lastArgumentID = kInvalidID;
	}

	return true;
}

bool ArgumentParser::TryParseValue(const char* argument, u64& lastArgumentID)
{
	std::string value(argument);
	const ValidValues& allValid = validValues[lastArgumentID];
	if (allValid.empty())
	{
		char first = argument[0];
		if (first >= '0' && first <= '9')
			argumentValues[lastArgumentID] = std::stoull(value);
		else
		{
			std::cerr << "Expected an unsigned integer value for '" << argumentNames[lastArgumentID] << "', but got '" << argument << "'" << std::endl;
			return false;
		}
	}
	else
	{
		auto begin = allValid.begin();
		auto end = allValid.end();
		auto it = std::find(begin, end, value);
		if (it != end)
		{
			argumentValues[lastArgumentID] = it - begin;
		}
		else
		{
			std::cerr << "Unknown value for parameter '" << argumentNames[lastArgumentID] << "'. Got '" << value << "', expected one of:";
			for (auto i = begin; i != end; ++i)
			{
				std::cerr << (i != begin ? ',' : ' ') << '\'' << *i << '\'';
			}
			std::cerr << std::endl;
			return false;
		}
	}

	lastArgumentID = kInvalidID;

	return true;
}
