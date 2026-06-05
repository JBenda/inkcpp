#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("run story with global variable", "[globals][runtime]")
{
	GIVEN("a story with global variables")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "GlobalStory.bin")};
		globals                globStore = ink->new_globals();
		runner                 thread    = ink->new_runner(globStore);

		WHEN("the story runs with default variable values")
		{
			std::string out = thread->getall();

			THEN("the output uses the default variable values")
			{
				REQUIRE(
				    out
				    == "My name is Jean Passepartout, but my friend's call me Jackie. I'm 23 years old.\nFoo:23\n"
				);
			}

			THEN("the globals store reflects the default values")
			{
				REQUIRE(*globStore->get<int32_t>("age") == 23);
				REQUIRE(*globStore->get<const char*>("friendly_name_of_player") == std::string{"Jackie"});
			}
		}

		WHEN("global variables are overridden before the story runs")
		{
			bool resi = globStore->set<int32_t>("age", 30);
			bool resc = globStore->set<const char*>("friendly_name_of_player", "Freddy");

			THEN("the set operations succeed")
			{
				REQUIRE(resi == true);
				REQUIRE(resc == true);
			}

			THEN("the output uses the overridden variable values")
			{
				std::string out = thread->getall();
				REQUIRE(
				    out
				    == "My name is Jean Passepartout, but my friend's call me Freddy. I'm 30 years old.\nFoo:30\n"
				);
				REQUIRE(*globStore->get<int32_t>("age") == 30);
				REQUIRE(*globStore->get<const char*>("friendly_name_of_player") == std::string{"Freddy"});
			}

			AND_WHEN("the story runs and a string is concatenated by the ink script")
			{
				thread->getall();

				THEN("the globals store reflects the concatenated string")
				{
					REQUIRE(*globStore->get<const char*>("concat") == std::string{"Foo:30"});
				}
			}
		}

		WHEN("a variable is accessed with the wrong type")
		{
			THEN("get returns no value")
			{
				REQUIRE(globStore->get<uint32_t>("age").has_value() == false);
			}

			THEN("set returns false") { REQUIRE(globStore->set<uint32_t>("age", 3) == false); }
		}

		WHEN("a variable name that does not exist is accessed")
		{
			THEN("get returns no value") { REQUIRE(globStore->get<int32_t>("foo").has_value() == false); }

			THEN("set returns false") { REQUIRE(globStore->set<int32_t>("foo", 3) == false); }
		}
	}
}
