#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("a story with bracketed choices and spaces can choose correctly", "[choices][runtime]")
{
	GIVEN("a story with bracketed choices")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "ChoiceBracketStory.bin")};
		runner                 thread = ink->new_runner();
		thread->getall();

		WHEN("the story is at the first choice point")
		{
			thread->getall();

			THEN("choices are available") { REQUIRE(thread->has_choices()); }

			AND_WHEN("choice 1 is made")
			{
				thread->choose(0);
				thread->getall();
				thread->getall();

				THEN("choices are still available after choice 1") { REQUIRE(thread->has_choices()); }
			}

			AND_WHEN("choice 2 is made")
			{
				thread->choose(1);
				thread->getall();

				THEN("choices are still available after choice 2") { REQUIRE(thread->has_choices()); }
			}
		}
	}
}
