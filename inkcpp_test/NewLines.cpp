#include "catch.hpp"
#include "../inkcpp_cl/test.h"

#include <story.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("a story has the proper line breaks", "[lines]")
{
	GIVEN("a story with line breaks")
	{
		inklecate("ink/LinesStory.ink", "LinesStory.tmp");
		ink::compiler::run("LinesStory.tmp", "LinesStory.bin");
		auto ink = story::from_file("LinesStory.bin");
		runner thread = ink->new_runner();
		WHEN("start thread")
		{
			THEN("thread can continue")
			{
				REQUIRE(thread->can_continue());
			}
			WHEN("consume lines")
			{
				std::string line1 = thread->getline();
				std::string line2 = thread->getline();
				std::string line3 = thread->getline();
				std::string line4 = thread->getline();
				THEN("lines are correct")
				{
					REQUIRE(line1 == "Line 1\n");
					REQUIRE(line2 == "Line 2\n");
					REQUIRE(line3 == "Line 3\n");
					REQUIRE(line4 == "Line 4\n");
				}
			}
			WHEN("consume lines with functions")
			{
				thread->move_to(ink::hash_string("Functions"));
				std::string line1 = thread->getline();
				std::string line2 = thread->getline();

				THEN("function lines are correct") {
					REQUIRE(line1 == "Function Line\n");
					REQUIRE(line2 == "Function Result\n");
				}
			}
			WHEN("consume lines with tunnels")
			{
				thread->move_to(ink::hash_string("Tunnels"));
				std::string line1 = thread->getline();
				std::string line2 = thread->getline();

				THEN("tunnel lines are correct") {
					REQUIRE(line1 == "Tunnel Line\n");
					REQUIRE(line2 == "Tunnel Result\n");
				}

				THEN("thread cannot continue")
				{
					REQUIRE(!thread->can_continue());
				}
			}
		}
	}
}
