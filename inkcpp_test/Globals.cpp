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
				REQUIRE(globStore->get<global_string<false>>("friendly_name_of_player").data() == std::string{"Jackie"});
			}
		}
		WHEN ("edit number")
		{
			globals globStore = ink->new_globals();
			runner thread = ink->new_runner(globStore);
			*globStore->get<int32_t>("age") = 30;
			globStore->get<global_string<true>>("friendly_name_of_player").set("Freddy");
			THEN("variable should contain new value")
			{
				REQUIRE(thread->getall() == "My name is Jean Passepartout, but my friend's call me Freddy. I'm 30 years old.\n");
				REQUIRE(*globStore->get<int32_t>("age") == 30);
				REQUIRE(globStore->get<global_string<false>>("friendly_name_of_player").data() == std::string{"Freddy"});
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
