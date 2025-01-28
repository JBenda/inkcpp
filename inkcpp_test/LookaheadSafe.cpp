#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("a story with external functions and glue", "[external]")
{
	GIVEN("the story")
	{
		auto ink = story::from_file(INK_TEST_RESOURCE_DIR "LookaheadSafe.bin");

		int  cnt = 0;
		auto foo = [&cnt]() {
			cnt += 1;
		};

		WHEN("the external function is safe for look-ahead")
		{
			auto thread = ink->new_runner();
			thread->bind("foo", foo, true);
			CHECK(thread->getline() == "Call1 glued to Call 2\n");
			CHECK(cnt == 3);
			CHECK(thread->getline() == "Call 3 is seperated\n");
			CHECK(cnt == 4);
		}
		WHEN("the external function is unsafe for look-ahead")
		{
			auto thread = ink->new_runner();
			thread->bind("foo", foo, false);
			CHECK(thread->getline() == "Call1\n");
			CHECK(cnt == 1);
			CHECK(thread->getline() == "glued to Call 2\n");
			CHECK(cnt == 2);
			CHECK(thread->getline() == "Call 3 is seperated\n");
			CHECK(cnt == 3);
		}
	}
}
