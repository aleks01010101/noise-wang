#include "ArgumentParserTests.hpp"

static const char* kDefaultArgument[] = {
	"defaultArg"
};

ArgumentParserSuite::ArgumentParserSuite()
	: TestSuite("ArgumentParser")
{
	REGISTER_TEST(ArgumentParser_DefaultArgument_ParseSuccessful);

	REGISTER_TEST(ArgumentParser_OneKnownArgument_ParseSuccessful);
	REGISTER_TEST(ArgumentParser_OneKnownArgumentShortName_ParseSuccessful);
	REGISTER_TEST(ArgumentParser_TwoKnownArgumentsOutOfOrder_ParseSuccessful);
	REGISTER_TEST(ArgumentParser_TwoKnownArgumentsOutOfOrderShortNames_ParseSuccessful);
	REGISTER_TEST(ArgumentParser_OneUnknownArgument_ParseFails);
	REGISTER_TEST(ArgumentParser_OneUnknownArgumentBeforeKnownArgument_ParseFails);
	REGISTER_TEST(ArgumentParser_OneUnknownArgumentAfterKnownArgument_ParseFails);

	REGISTER_TEST(ArgumentParser_ArgumentWithoutValidValues_Zero);
	REGISTER_TEST(ArgumentParser_ArgumentWithEmptyValidValue_Zero);
	REGISTER_TEST(ArgumentParser_ArgumentWithValidValues_Zero);
	REGISTER_TEST(ArgumentParser_ArgumentWithoutValidValuesWithCustomDefault_CustomDefault);
	REGISTER_TEST(ArgumentParser_ArgumentWithEmptyValidValueWithCustomDefault_CustomDefault);
	REGISTER_TEST(ArgumentParser_ArgumentWithValidValuesWithCustomDefault_CustomDefault);

	REGISTER_TEST(ArgumentParser_ArgumentWithoutValidValues_ValueAsIs);
	REGISTER_TEST(ArgumentParser_ArgumentWithValidValues_ValueIndex);
	REGISTER_TEST(ArgumentParser_ArgumentWithoutValidValuesNotANumber_ParseFails);
	REGISTER_TEST(ArgumentParser_ArgumentWithValidValuesValueNotFromSet_ParseFails);
}

// Category 1: default argument parsing

// 1.1: default argument supplied -> success
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_DefaultArgument_ParseSuccessful)
{
	const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

	Check(parser.Parse(numArguments, kDefaultArgument));
}

// Category 2: argument name parsing

// 2.1: single known argument -> success
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_OneKnownArgument_ParseSuccessful)
{
	const char* arguments[] = {
		"defaultArg",
		"--knownArg"
	};
	const i32 numArguments = sizeof(arguments) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", {""});
	Check(parser.Parse(numArguments, arguments));
}

// 2.2: single known argument, short name -> success
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_OneKnownArgumentShortName_ParseSuccessful)
{
	const char* arguments[] = {
		"defaultArg",
		"-k"
	};
	const i32 numArguments = sizeof(arguments) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", { "" });
	Check(parser.Parse(numArguments, arguments));
}

// 2.3: two known arguments, out of order -> success
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_TwoKnownArgumentsOutOfOrder_ParseSuccessful)
{
	const char* arguments[] = {
		"defaultArg",
		"--secondKnownArg",
		"--firstKnownArg"
	};
	const i32 numArguments = sizeof(arguments) / sizeof(const char*);

	parser.AddKnownArgument("firstKnownArg", "f", { "" });
	parser.AddKnownArgument("secondKnownArg", "s", { "" });
	Check(parser.Parse(numArguments, arguments));
}

// 2.4: two known arguments, out of order, short names -> success
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_TwoKnownArgumentsOutOfOrderShortNames_ParseSuccessful)
{
	const char* arguments[] = {
		"defaultArg",
		"-s",
		"-f"
	};
	const i32 numArguments = sizeof(arguments) / sizeof(const char*);

	parser.AddKnownArgument("firstKnownArg", "f", { "" });
	parser.AddKnownArgument("secondKnownArg", "s", { "" });
	Check(parser.Parse(numArguments, arguments));
}

// 2.5: unknown argument -> fail
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_OneUnknownArgument_ParseFails)
{
	const char* arguments[] = {
		"defaultArg",
		"--unknownArg"
	};
	const i32 numArguments = sizeof(arguments) / sizeof(const char*);

	Check(!parser.Parse(numArguments, arguments));
}

// 2.6: unknown argument, then known argument -> fail
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_OneUnknownArgumentBeforeKnownArgument_ParseFails)
{
	const char* arguments[] = {
		"defaultArg",
		"--unknownArg",
		"--knownArg"
	};
	const i32 numArguments = sizeof(arguments) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", { "" });
	Check(!parser.Parse(numArguments, arguments));
}

// 2.7: known argument, then unknown argument -> fail
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_OneUnknownArgumentAfterKnownArgument_ParseFails)
{
	const char* arguments[] = {
		"defaultArg",
		"--knownArg",
		"--unknownArg"
	};
	const i32 numArguments = sizeof(arguments) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", { "" });
	Check(!parser.Parse(numArguments, arguments));
}

// Category 3: argument value defaults

// 3.1: argument without valid values -> 0
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_ArgumentWithoutValidValues_Zero)
{
	const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", {});
	parser.Parse(numArguments, kDefaultArgument);

	CheckEqual(0ull, parser.GetValue("knownArg"));
}

// 3.2: argument with one empty valid value -> 0
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_ArgumentWithEmptyValidValue_Zero)
{
	const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", { "" });
	parser.Parse(numArguments, kDefaultArgument);

	CheckEqual(0ull, parser.GetValue("knownArg"));
}

// 3.3: argument with several valid values -> 0
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_ArgumentWithValidValues_Zero)
{
	const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", { "foo", "bar" });
	parser.Parse(numArguments, kDefaultArgument);

	CheckEqual(0ull, parser.GetValue("knownArg"));
}

// 3.4: argument without valid values, custom default -> custom default
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_ArgumentWithoutValidValuesWithCustomDefault_CustomDefault)
{
	const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", {}, 123ull);
	parser.Parse(numArguments, kDefaultArgument);

	CheckEqual(123ull, parser.GetValue("knownArg"));
}

// 3.5: argument with one empty valid value, custom default -> custom default
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_ArgumentWithEmptyValidValueWithCustomDefault_CustomDefault)
{
	const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", { "" }, 1ull);
	parser.Parse(numArguments, kDefaultArgument);

	CheckEqual(1ull, parser.GetValue("knownArg"));
}

// 3.6: argument with several valid values, custom default -> custom default
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_ArgumentWithValidValuesWithCustomDefault_CustomDefault)
{
	const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", { "foo", "bar" }, 1ull);
	parser.Parse(numArguments, kDefaultArgument);

	CheckEqual(1ull, parser.GetValue("knownArg"));
}

// Category 4: specified argument value

// 4.1: no valid values -> no check
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_ArgumentWithoutValidValues_ValueAsIs)
{
	const char* arguments[] = {
		"defaultArg",
		"--knownArg",
		"987654321"
	};
	const i32 numArguments = sizeof(arguments) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", {});
	Check(parser.Parse(numArguments, arguments));
	CheckEqual(987654321ull, parser.GetValue("knownArg"));
}

// 4.2: several valid values -> selects value index
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_ArgumentWithValidValues_ValueIndex)
{
	const char* arguments[] = {
		"defaultArg",
		"--knownArg",
		"bar"
	};
	const i32 numArguments = sizeof(arguments) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", { "foo", "bar" });
	Check(parser.Parse(numArguments, arguments));
	CheckEqual(1ull, parser.GetValue("knownArg"));
}

// 4.3: no valid values, not a number -> fail
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_ArgumentWithoutValidValuesNotANumber_ParseFails)
{
	const char* arguments[] = {
		"defaultArg",
		"--knownArg",
		"bar"
	};
	const i32 numArguments = sizeof(arguments) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", {});
	Check(!parser.Parse(numArguments, arguments));
}

// 4.4. several valid values, value not in set -> fail
IMPLEMENT_TEST_FIXTURE(ArgumentParserSuite, ArgumentParser_ArgumentWithValidValuesValueNotFromSet_ParseFails)
{
	const char* arguments[] = {
		"defaultArg",
		"--knownArg",
		"barge"
	};
	const i32 numArguments = sizeof(arguments) / sizeof(const char*);

	parser.AddKnownArgument("knownArg", "k", { "foo", "bar" });
	Check(!parser.Parse(numArguments, arguments));
}
