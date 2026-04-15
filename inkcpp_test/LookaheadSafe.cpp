#include "catch.hpp"

#include <../runner_impl.h>
#include <compiler.h>
#include <globals.h>
#include <runner.h>
#include <story.h>

using namespace ink::runtime;

SCENARIO("a story with external functions and glue", "[external]")
{
	GIVEN("the story")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "LookaheadSafe.bin")};

		int  cnt = 0;
		auto foo = [&cnt]() {
			cnt += 1;
		};

		WHEN("the external function is safe for look-ahead")
		{
			auto              thread = ink->new_runner().cast<internal::runner_impl>();
			std::stringstream commands;
			thread->set_debug_enabled(&commands);
			thread->bind("foo", foo, true);
			CHECK(thread->getline() == "Call1 glued to Call 2\n");
			std::string c = commands.str();
			CHECK(cnt == 3);
			REQUIRE(thread->getline() == "Call 3 is separated\n");
			CHECK(cnt == 4);
		}
		WHEN("the external function is unsafe for look-ahead")
		{
			auto              thread = ink->new_runner().cast<internal::runner_impl>();
			std::stringstream commands;
			thread->set_debug_enabled(&commands);
			thread->bind("foo", foo, false);
			CHECK(thread->getline() == "Call1\n");
			std::string c = commands.str();
			CHECK(cnt == 1);
			CHECK(thread->getline() == "glued to Call 2\n");
			CHECK(cnt == 2);
			CHECK(thread->getline() == "Call 3 is separated\n");
			CHECK(cnt == 3);
		}
	}
}
