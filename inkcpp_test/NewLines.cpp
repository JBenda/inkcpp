#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

std::unique_ptr<story> lines_ink{story::from_file(INK_TEST_RESOURCE_DIR "LinesStory.bin")};
runner                 lines_thread = lines_ink->new_runner();

SCENARIO("a story has the proper line breaks", "[lines]")
{
	GIVEN("a story with line breaks")
	{
		WHEN("starting thread")
		{
			THEN("thread can continue") { REQUIRE(lines_thread->can_continue()); }
			THEN("consume lines")
			{
				CHECK(lines_thread->getline() == "Line 1\n");
				CHECK(lines_thread->getline() == "Line 2\n");
				CHECK(lines_thread->getline() == "Line 3\n");
				CHECK(lines_thread->getline() == "Line 4\n");
			}
		}
		WHEN("running functions")
		{
			lines_thread->move_to(ink::hash_string("Functions"));
			CHECK(lines_thread->getline() == "Function Line\n");

			THEN("consume function result") { CHECK(lines_thread->getline() == "Function Result\n"); }
		}
		WHEN("consuming lines with tunnels")
		{
			lines_thread->move_to(ink::hash_string("Tunnels"));

			THEN("tunnel lines are correct")
			{
				CHECK(lines_thread->getline() == "Tunnel Line\n");
				CHECK(lines_thread->getline() == "Tunnel Result\n");
				CHECK(lines_thread->getline() == "");
				CHECK_FALSE(lines_thread->can_continue());
			}
		}
		WHEN("ignoring functions when applying glue")
		{
			lines_thread->move_to(ink::hash_string("ignore_functions_when_applying_glue"));
			CHECK(lines_thread->getline() == "\"I don't see why,\" I reply.\n");
		}
	}
	GIVEN("a complex story")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "TheIntercept.bin")};
		runner                 thread = ink->new_runner();
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
