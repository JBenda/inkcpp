#include "catch.hpp"
#include "../inkcpp_cl/test.cpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("run story with global variable", "[global variables]")
{
	GIVEN ("a story with global variables")
	{
		inklecate("ink/GlobalStory.ink", "GlobalsStory.tmp");
		ink::compiler::run("GlobalsStory.tmp", "GlobalsStory.bin", "GlobalsStory.bin.str");
		auto ink = story::create("GlobalsStory.bin", "GlobalsStory.bin.str");
		globals globStore = ink->new_globals();
		runner thread = ink->new_runner(globStore);

		WHEN( "just runs")
		{
			THEN("variables should contain values as in inkScript")
			{
				REQUIRE(thread->getall() == "My name is Jean Passepartout, but my friend's call me Jackie. I'm 23 years old.\nFoo:23\n");
				REQUIRE(*globStore->get<int32_t>("age") == 23);
				REQUIRE(*globStore->get<const char*>("friendly_name_of_player") == std::string{"Jackie"});
			}
		}
		WHEN ("edit number")
		{
			bool resi
				= globStore->set<int32_t>("age", 30);
			bool resc
				= globStore->set<const char*>("friendly_name_of_player", "Freddy");
			THEN("execution should success")
			{
				REQUIRE(resi == true);
				REQUIRE(resc == true);
			}
			THEN("variable should contain new value")
			{
				REQUIRE(thread->getall() == "My name is Jean Passepartout, but my friend's call me Freddy. I'm 30 years old.\nFoo:30\n");
				REQUIRE(*globStore->get<int32_t>("age") == 30);
				REQUIRE(*globStore->get<const char*>("friendly_name_of_player") == std::string{"Freddy"});
			}
			WHEN ("something added to string")
			{
				// concat in GlobalsStory.ink
				thread->getall();
				THEN("get should return the whole string")
				{
					REQUIRE(*globStore->get<const char*>("concat") == std::string{"Foo:30"});
				}
			}
		}
		WHEN ("name or type not exist")
		{
			auto wrongType = globStore->get<uint32_t>("age");
			auto notExistingName = globStore->get<int32_t>("foo");
			THEN("should return nullptr")
			{
				REQUIRE(wrongType.has_value() == false);
				REQUIRE(notExistingName.has_value() == false);
			}

			bool rest = globStore->set<uint32_t>("age", 3);
			bool resn = globStore->set<int32_t>("foo", 3);
			THEN("should return false")
			{
				REQUIRE(rest == false);
				REQUIRE(resn == false);
			}
		}
	}
}
