#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO(
    "a story with a bracketed choice as a second choice, and then a third choice, chooses properly",
    "[choices][runtime]"
)
{
	GIVEN("a story with brackets and nested choices")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR
		                                            "ThirdTierChoiceAfterBracketsStory.bin")};
		runner                 thread = ink->new_runner();

		WHEN("the story starts")
		{
			thread->getall();

			THEN("the first tier choices are presented") { REQUIRE(thread->has_choices()); }

			AND_WHEN("the first choice is made")
			{
				thread->choose(0);
				thread->getall();

				THEN("the second tier choices are presented") { REQUIRE(thread->has_choices()); }

				AND_WHEN("the second choice is made")
				{
					thread->choose(0);
					thread->getall();

					THEN("the third tier choices are presented") { REQUIRE(thread->has_choices()); }

					AND_WHEN("the third choice is made")
					{
						thread->choose(0);
						thread->getall();

						THEN("the story ends with no further choices") { REQUIRE_FALSE(thread->has_choices()); }
					}
				}
			}
		}
	}
}
