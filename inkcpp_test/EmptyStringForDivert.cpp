#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO(
    "a story with a white space infront of an conditional Divert", "[output][regression][runtime]"
)
{
	// based on https://github.com/JBenda/inkcpp/issues/71
	GIVEN("a story with a conditional divert")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "EmptyStringForDivert.bin")};
		runner                 thread = ink->new_runner();

		WHEN("the story starts and the first choice is made")
		{
			thread->getall();
			REQUIRE(thread->has_choices()); // guard
			thread->choose(0);
			std::string line = thread->getall();

			THEN("'This displays first' is printed and another choice is offered")
			{
				REQUIRE(line == "This displays first\n");
				REQUIRE(thread->has_choices());
			}

			AND_WHEN("the second choice is made")
			{
				REQUIRE(thread->has_choices()); // guard
				thread->choose(0);
				std::string line2 = thread->getall();

				THEN("the continuation text is printed and another choice is offered")
				{
					REQUIRE(line2 == "This is the continuation.\n");
					REQUIRE(thread->has_choices());
				}

				AND_WHEN("the third choice is made")
				{
					REQUIRE(thread->has_choices()); // guard
					thread->choose(0);
					std::string line3 = thread->getall();

					THEN("empty output is produced and the story ends")
					{
						REQUIRE(line3 == "");
						REQUIRE_FALSE(thread->has_choices());
					}
				}
			}
		}
	}
}
