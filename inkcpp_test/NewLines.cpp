#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

auto   _ink    = story::from_file(INK_TEST_RESOURCE_DIR "LinesStory.bin");
runner _thread = _ink->new_runner();

SCENARIO("a story has the proper line breaks", "[lines]")
{
	GIVEN("a story with line breaks")
	{
		WHEN("starting thread")
		{
			THEN("thread can continue")
			{
				REQUIRE(_thread->can_continue());
			}
			THEN("consume lines")
			{
				CHECK(_thread->getline() == "Line 1\n");
				CHECK(_thread->getline() == "Line 2\n");
				CHECK(_thread->getline() == "Line 3\n");
				CHECK(_thread->getline() == "Line 4\n");
			}
		}
		WHEN("running functions")
		{
			_thread->move_to(ink::hash_string("Functions"));
			CHECK(_thread->getline() == "Function Line\n");

			THEN("consume function result")
			{
				CHECK(_thread->getline() == "Function Result");
			}
		}
		WHEN("conuming lines with tunnels")
		{
			_thread->move_to(ink::hash_string("Tunnels"));

			THEN("tunnel lines are correct")
			{
				CHECK(_thread->getline() == "Tunnel Line\n");
				CHECK(_thread->getline() == "Tunnel Result\n");
				CHECK(_thread->getline() == "");
				CHECK_FALSE(_thread->can_continue());
			}
		}
	}
	GIVEN("a complex story")
	{
		auto   ink    = story::from_file(INK_TEST_RESOURCE_DIR "TheIntercept.bin");
		runner thread = ink->new_runner();
		// based on issue #82
		WHEN("run sequence 1 3 3 3 2 3")
		{
			for (int i : {1, 3, 3, 3, 2, 3}) {
				thread->getall();
				thread->choose(i - 1);
			}
			std::string text = thread->getall();
			THEN("no newline before dot") { REQUIRE(text == "\"I don't see why,\" I reply.\n"); }
		}
	}
}
