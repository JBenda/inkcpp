#include "catch.hpp"

#include <../runner_impl.h>
#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("a story with an external function evaluates the function at the right time", "[story]")
{
	GIVEN("a story with an external function")
	{
		auto ink    = story::from_file(INK_TEST_RESOURCE_DIR "ExternalFunctionsExecuteProperly.bin");
		auto   thread = ink->new_runner().cast<internal::runner_impl>();

		std::stringstream debug;
		thread->set_debug_enabled(&debug);

		int line_count = 0;
		thread->bind("GET_LINE_COUNT", [&line_count]() { return line_count; });

		WHEN("run thread")
		{
			THEN("thread has correct line counts")
			{
				while (thread->can_continue()) {
					auto line = thread->getline();
					REQUIRE(line == "Line count: " + std::to_string(line_count) + "\n");
					line_count++;
				}
			}
		}
	}
}
