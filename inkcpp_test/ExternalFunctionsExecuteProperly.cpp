#include "catch.hpp"
#include "../inkcpp_cl/test.h"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("a story with an external function evaluates the function at the right time", "[story]")
{
	GIVEN("a story with an external function")
	{
		inklecate("ink/ExternalFunctionsExecuteProperly.ink", "ExternalFunctionsExecuteProperly.tmp");
		ink::compiler::run("ExternalFunctionsExecuteProperly.tmp", "ExternalFunctionsExecuteProperly.bin");
		auto   ink    = story::from_file("ExternalFunctionsExecuteProperly.bin");
		runner thread = ink->new_runner();

		int line_count = 0;


		thread->bind("GET_LINE_COUNT", [&line_count]() {
			return line_count;
		});

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
