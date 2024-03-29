#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO(
    "a story with a bracketed choice as a second choice, and then a third choice, chooses properly",
    "[choices]"
)
{
	GIVEN("a story with brackets and nested choices")
	{
		auto   ink    = story::from_file(INK_TEST_RESOURCE_DIR "ThirdTierChoiceAfterBracketsStory.bin");
		runner thread = ink->new_runner();

		WHEN("start thread")
		{
			THEN("thread doesn't error")
			{
				thread->getall();
				REQUIRE(thread->has_choices());
				thread->choose(0);
				thread->getall();
				REQUIRE(thread->has_choices());
				thread->choose(0);
				thread->getall();
				REQUIRE(thread->has_choices());
				thread->choose(0);
				thread->getall();
				REQUIRE(! thread->has_choices());
			}
		}
	}
}
