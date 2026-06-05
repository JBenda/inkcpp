#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>

using namespace ink::runtime;

SCENARIO("a story has the proper line breaks", "[output][runtime]")
{
	GIVEN("a story with line breaks")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "LinesStory.bin")};
		runner                 thread = ink->new_runner();

		WHEN("the thread starts")
		{
			THEN("the thread can continue") { REQUIRE(thread->can_continue()); }
		}

		WHEN("four lines are consumed in order")
		{
			std::string line1 = thread->getline();
			std::string line2 = thread->getline();
			std::string line3 = thread->getline();
			std::string line4 = thread->getline();

			THEN("each line matches the expected content")
			{
				CHECK(line1 == "Line 1\n");
				CHECK(line2 == "Line 2\n");
				CHECK(line3 == "Line 3\n");
				CHECK(line4 == "Line 4\n");
			}
		}

		WHEN("the runner jumps to the Functions knot")
		{
			thread->move_to(ink::hash_string("Functions"));
			std::string line1 = thread->getline();
			std::string line2 = thread->getline();

			THEN("the function line and its result are output correctly")
			{
				CHECK(line1 == "Function Line\n");
				CHECK(line2 == "Function Result\n");
			}
		}

		WHEN("the runner jumps to the Tunnels knot")
		{
			thread->move_to(ink::hash_string("Tunnels"));
			std::string line1 = thread->getline();
			std::string line2 = thread->getline();
			std::string line3 = thread->getline();

			THEN("the tunnel lines are correct and the story ends")
			{
				CHECK(line1 == "Tunnel Line\n");
				CHECK(line2 == "Tunnel Result\n");
				CHECK(line3 == "");
				CHECK_FALSE(thread->can_continue());
			}
		}

		WHEN("the runner jumps to the ignore_functions_when_applying_glue knot")
		{
			thread->move_to(ink::hash_string("ignore_functions_when_applying_glue"));
			std::string line = thread->getline();

			THEN("functions are correctly ignored when applying glue")
			{
				CHECK(line == "\"I don't see why,\" I reply.\n");
			}
		}
	}

	GIVEN("a complex story")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "TheIntercept.bin")};
		runner                 thread = ink->new_runner();

		WHEN("the sequence 1 3 3 3 2 3 is chosen")
		{
			for (int i : {1, 3, 3, 3, 2, 3}) {
				thread->getall();
				thread->choose(i - 1);
			}
			std::string text = thread->getall();

			THEN("there is no spurious newline before the final punctuation")
			{
				REQUIRE(text == "\"I don't see why,\" I reply.\n");
			}
		}
	}
}
