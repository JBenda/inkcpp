#include "catch.hpp"
#include "../inkcpp_cl/test.h"

#include <story.h>
#include <runner.h>
#include <globals.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("run a story with lists", "[lists]")
{
	GIVEN("a story with multi lists")
	{
		inklecate("ink/ListStory.ink", "ListStory.tmp");
		ink::compiler::run("ListStory.tmp", "ListStory.bin");
		auto ink = story::from_file("ListStory.bin");
		runner thread = ink->new_runner();

		WHEN("just run")
		{
			std::string out = thread->getall();
			THEN("should output expected")
			{
			REQUIRE(out == "TODO: replace");
			}
		}
	}
}
