#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>

using namespace ink::runtime;

SCENARIO("run story with global variable", "[global variables]")
{
	GIVEN ("a story with global variables")
	{
		story* ink = story::from_file("ink/GlobalStory.bin");


		WHEN( "just runs")
		{
			globals globStore = ink->new_globals();
			runner thread = ink->new_runner(globStore);
			THEN("variables should contain values as in inkScript")
			{
				REQUIRE(thread->getall() == "My name is Jean Passepartout, but my friend's call me Jackie. I'm 23 years old.\n");
				REQUIRE(*globStore->get<int32_t>("age") == 23);
			}
		}
		WHEN ("edit number")
		{
			globals globStore = ink->new_globals();
			runner thread = ink->new_runner(globStore);
			*globStore->get<int32_t>("age") = 30;
			THEN("variable should contain new value")
			{
				REQUIRE(thread->getall() == "My name is Jean Passepartout, but my friend's call me Jackie. I'm 30 years old.\n");
				REQUIRE(*globStore->get<int32_t>("age") == 30);
			}
		}
		WHEN ("name or type not exist")
		{
			globals globStore = ink->new_globals();
			runner thread = ink->new_runner(globStore);
			auto wrongType = globStore->get<uint32_t>("age");
			auto notExistingName = globStore->get<int32_t>("foo");
			THEN("should return nullptr")
			{
				REQUIRE(wrongType == nullptr);
				REQUIRE(notExistingName == nullptr);
			}
		}
	}
}
