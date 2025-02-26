#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("A story with external functions and glue", "[external]")
{
	GIVEN("The story")
	{
		auto ink = story::from_file(INK_TEST_RESOURCE_DIR "LookaheadSafe.bin");

		int  cnt = 0;
		auto foo = [&cnt]() {
			cnt += 1;
		};
		WHEN("the function in lookahead save")
		{
			auto thread = ink->new_runner();
			thread->bind("foo", foo, true);
			std::string out = thread->getline();
			REQUIRE(cnt == 3);
			REQUIRE(out == "Call1 glued to Call 2\n");
			out = thread->getline();
			REQUIRE(out == "Call 3 is separated\n");
			REQUIRE(cnt == 4);
		}
		WHEN("the function is not lookahead save")
		{
			auto thread = ink->new_runner();
			thread->bind("foo", foo, false);
			std::string out = thread->getline();
			REQUIRE(out == "Call1\n");
			REQUIRE(cnt == 1);
			out = thread->getline();
			REQUIRE(out == "glued to Call 2\n");
			REQUIRE(cnt == 2);
			out = thread->getline();
			REQUIRE(out == "Call 3 is separated\n");
			REQUIRE(cnt == 3);
		}
	}
}
