#pragma once

#include "ArgumentParser.hpp"

#include "testing/TestFixture.hpp"
#include "testing/TestSuite.hpp"

class ArgumentParserSuite
	: public TestSuite
{
public:
	struct Fixture
	{
		ArgumentParser parser;
	};

	ArgumentParserSuite();

	// Category 1: default argument parsing
	// 1.1: default argument supplied -> success
	DECLARE_TEST_FIXTURE(ArgumentParser_DefaultArgument_ParseSuccessful);

	// Category 2: argument name parsing
	// 2.1: single known argument -> success
	DECLARE_TEST_FIXTURE(ArgumentParser_OneKnownArgument_ParseSuccessful);
	// 2.2: single known argument, short name -> success
	DECLARE_TEST_FIXTURE(ArgumentParser_OneKnownArgumentShortName_ParseSuccessful);
	// 2.3: two known arguments, out of order -> success
	DECLARE_TEST_FIXTURE(ArgumentParser_TwoKnownArgumentsOutOfOrder_ParseSuccessful);
	// 2.4: two known arguments, out of order, short names -> success
	DECLARE_TEST_FIXTURE(ArgumentParser_TwoKnownArgumentsOutOfOrderShortNames_ParseSuccessful);
	// 2.5: unknown argument -> fail
	DECLARE_TEST_FIXTURE(ArgumentParser_OneUnknownArgument_ParseFails);
	// 2.6: unknown argument, then known argument -> fail
	DECLARE_TEST_FIXTURE(ArgumentParser_OneUnknownArgumentBeforeKnownArgument_ParseFails);
	// 2.7: known argument, then unknown argument -> fail
	DECLARE_TEST_FIXTURE(ArgumentParser_OneUnknownArgumentAfterKnownArgument_ParseFails);

	// Category 3: argument value defaults
	// 3.1: argument without valid values -> 0
	DECLARE_TEST_FIXTURE(ArgumentParser_ArgumentWithoutValidValues_Zero);
	// 3.2: argument with one empty valid value -> 0
	DECLARE_TEST_FIXTURE(ArgumentParser_ArgumentWithEmptyValidValue_Zero);
	// 3.3: argument with several valid values -> 0
	DECLARE_TEST_FIXTURE(ArgumentParser_ArgumentWithValidValues_Zero);
	// 3.4: argument without valid values, custom default -> custom default
	DECLARE_TEST_FIXTURE(ArgumentParser_ArgumentWithoutValidValuesWithCustomDefault_CustomDefault);
	// 3.5: argument with one empty valid value, custom default -> custom default
	DECLARE_TEST_FIXTURE(ArgumentParser_ArgumentWithEmptyValidValueWithCustomDefault_CustomDefault);
	// 3.6: argument with several valid values, custom default -> custom default
	DECLARE_TEST_FIXTURE(ArgumentParser_ArgumentWithValidValuesWithCustomDefault_CustomDefault);

	// Category 4: specified argument value
	// 4.1: no valid values -> no check
	DECLARE_TEST_FIXTURE(ArgumentParser_ArgumentWithoutValidValues_ValueAsIs);
	// 4.2: several valid values -> selects value index
	DECLARE_TEST_FIXTURE(ArgumentParser_ArgumentWithValidValues_ValueIndex);
	// 4.3: no valid values, not a number -> fail
	DECLARE_TEST_FIXTURE(ArgumentParser_ArgumentWithoutValidValuesNotANumber_ParseFails);
	// 4.4. several valid values, value not in set -> fail
	DECLARE_TEST_FIXTURE(ArgumentParser_ArgumentWithValidValuesValueNotFromSet_ParseFails);
};
