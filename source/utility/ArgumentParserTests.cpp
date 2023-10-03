#include "ArgumentParser.hpp"

#include "testing/Test.hpp"
#include "testing/TestFixture.hpp"
#include "testing/TestRunner.hpp"
#include "testing/TestSuite.hpp"

// Category 1: default argument parsing
// 1.1: default argument supplied -> success
// Category 2: argument name parsing
// 2.1: single known argument -> success
// 2.2: single known argument, short name -> success
// 2.3: two known arguments, out of order -> success
// 2.4: two known arguments, out of order, short names -> success
// 2.5: unknown argument -> fail
// 2.6: unknown argument, then known argument -> fail
// 2.7: known argument, then unknown argument -> fail
// Category 3: argument value defaults
// 3.1: argument without valid values -> 0
// 3.2: argument with one empty valid value -> 0
// 3.3: argument with several valid values -> 0
// 3.4: argument without valid values, custom default -> custom default
// 3.5: argument with one empty valid value, custom default -> custom default
// 3.6: argument with several valid values, custom default -> custom default
// Category 4: specified argument value
// 4.1: no valid values -> no check
// 4.2: several valid values -> selects value index
// 4.3: no valid values, not a number -> fail
// 4.4. several valid values, value not in set -> fail

struct ArgumentParserFixture
{
	ArgumentParser parser;
};

static const char* kDefaultArgument[] = {
	"defaultArg"
};

// Category 1: default argument parsing
TEST_SUITE(ArgumentParser_DefaultArgumentParsing)
{
	// 1.1: default argument supplied -> success
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_DefaultArgument_ParseSuccessful)
	{
		const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

		Check(parser.Parse(numArguments, kDefaultArgument));
	}
}

// Category 2: argument name parsing
TEST_SUITE(ArgumentParser_ArgumentNameParsing)
{
	// 2.1: single known argument -> success
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_OneKnownArgument_ParseSuccessful)
	{
		const char* arguments[] = {
			"defaultArg",
			"--knownArg"
		};
		const i32 numArguments = sizeof(arguments) / sizeof(const char*);

		parser.AddKnownArgument("knownArg", "k", { "" });
		Check(parser.Parse(numArguments, arguments));
	}

	// 2.2: single known argument, short name -> success
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_OneKnownArgumentShortName_ParseSuccessful)
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
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_TwoKnownArgumentsOutOfOrder_ParseSuccessful)
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
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_TwoKnownArgumentsOutOfOrderShortNames_ParseSuccessful)
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
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_OneUnknownArgument_ParseFails)
	{
		const char* arguments[] = {
			"defaultArg",
			"--unknownArg"
		};
		const i32 numArguments = sizeof(arguments) / sizeof(const char*);

		Check(!parser.Parse(numArguments, arguments));
	}

	// 2.6: unknown argument, then known argument -> fail
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_OneUnknownArgumentBeforeKnownArgument_ParseFails)
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
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_OneUnknownArgumentAfterKnownArgument_ParseFails)
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
}

// Category 3: argument value defaults
TEST_SUITE(ArgumentParser_ArgumentValueDefaults)
{
	// 3.1: argument without valid values -> 0
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_ArgumentWithoutValidValues_ValueIsZero)
	{
		const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

		parser.AddKnownArgument("knownArg", "k", {});
		parser.Parse(numArguments, kDefaultArgument);

		CheckEqual(0ull, parser.GetValue("knownArg"));
	}

	// 3.2: argument with one empty valid value -> 0
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_ArgumentWithEmptyValidValue_ValueIsZero)
	{
		const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

		parser.AddKnownArgument("knownArg", "k", { "" });
		parser.Parse(numArguments, kDefaultArgument);

		CheckEqual(0ull, parser.GetValue("knownArg"));
	}

	// 3.3: argument with several valid values -> 0
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_ArgumentWithValidValues_ValueIsZero)
	{
		const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

		parser.AddKnownArgument("knownArg", "k", { "foo", "bar" });
		parser.Parse(numArguments, kDefaultArgument);

		CheckEqual(0ull, parser.GetValue("knownArg"));
	}

	// 3.4: argument without valid values, custom default -> custom default
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_ArgumentWithoutValidValuesWithCustomDefault_ValueIsCustomDefault)
	{
		const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

		parser.AddKnownArgument("knownArg", "k", {}, 123ull);
		parser.Parse(numArguments, kDefaultArgument);

		CheckEqual(123ull, parser.GetValue("knownArg"));
	}

	// 3.5: argument with one empty valid value, custom default -> custom default
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_ArgumentWithEmptyValidValueWithCustomDefault_ValueIsCustomDefault)
	{
		const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

		parser.AddKnownArgument("knownArg", "k", { "" }, 1ull);
		parser.Parse(numArguments, kDefaultArgument);

		CheckEqual(1ull, parser.GetValue("knownArg"));
	}

	// 3.6: argument with several valid values, custom default -> custom default
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_ArgumentWithValidValuesWithCustomDefault_ValueIsCustomDefault)
	{
		const i32 numArguments = sizeof(kDefaultArgument) / sizeof(const char*);

		parser.AddKnownArgument("knownArg", "k", { "foo", "bar" }, 1ull);
		parser.Parse(numArguments, kDefaultArgument);

		CheckEqual(1ull, parser.GetValue("knownArg"));
	}
}

// Category 4: specified argument value
TEST_SUITE(ArgumentParser_SpecifiedArgumentValue)
{
	// 4.1: no valid values -> no check
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_ArgumentWithoutValidValues_ValueAsIs)
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
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_ArgumentWithValidValues_ValueIndex)
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
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_ArgumentWithoutValidValuesNotANumber_ParseFails)
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
	TEST_FIXTURE(ArgumentParserFixture, ArgumentParser_ArgumentWithValidValuesValueNotFromSet_ParseFails)
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
}
