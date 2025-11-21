#include "catch.hpp"
#include "snapshot.h"
#include "system.h"

#include <story.h>
#include <globals.h>
#include <choice.h>
#include <runner.h>
#include <compiler.h>

#include <sstream>

using namespace ink::runtime;

SCENARIO("string_table fill up #97", "[fixes]")
{
	GIVEN("story murder_scene")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "murder_scene.bin")};
		globals                globStore = ink->new_globals();
		runner                 main      = ink->new_runner(globStore);

		WHEN("Run first choice 50 times")
		{
			std::string story = "";
			main->getall();
			while (main->has_choices()) {
				main->choose(0);
				story += main->getall();
			}
			THEN("string table should still have room")
			{
				REQUIRE(story.length() == 3082);
				// TEST string table size
			}
		}
	}
}

SCENARIO("unknown command _ #109", "[fixes]")
{
	GIVEN("story")
	{
		std::stringstream ss;
		ss << "{\"inkVersion\":21,\"root\":[[\"ev\",{\"VAR?\":\"boolvar\"},\"out\",\"/"
		      "ev\",\"\\n\",\"ev\",{\"VAR?\":\"boolvar\"},\"_\",\"out\",\"/"
		      "ev\",\"\\n\",\"ev\",{\"VAR?\":\"boolvar\"},\"_\",\"/"
		      "ev\",[{\"->\":\".^.b\",\"c\":true},{\"b\":[\"^ first boolvar "
		      "\",{\"->\":\"0.16\"},null]}],\"nop\",\"\\n\",[\"ev\",{\"VAR?\":\"boolvar\"},\"/"
		      "ev\",{\"->\":\".^.b\",\"c\":true},{\"b\":[\"\\n\",\"^second "
		      "boolvar\",\"\\n\",{\"->\":\"0.19\"},null]}],\"nop\",\"\\n\",\"end\",[\"done\",{\"#n\":"
		      "\"g-0\"}],null],\"done\",{\"global "
		      "decl\":[\"ev\",true,{\"VAR=\":\"boolvar\"},\"/ev\",\"end\",null]}],\"listDefs\":{}}";

		WHEN("Run")
		{
			std::stringstream                  out;
			ink::compiler::compilation_results res;
			ink::compiler::run(ss, out, &res);
			unsigned char* data;
			std::string    out_str = out.str();
			data                   = new unsigned char[out_str.size()];
			for (size_t i = 0; i < out_str.size(); ++i) {
				data[i] = out_str[i];
			}
			std::unique_ptr<story> ink{story::from_binary(data, out_str.size())};
			globals                globStore = ink->new_globals();
			runner                 main      = ink->new_runner(globStore);
			std::string            story     = main->getall();
			THEN("expect correct output")
			{
				REQUIRE(res.warnings.size() == 0);
				REQUIRE(res.errors.size() == 0);
				REQUIRE(
				    story ==
				    R"(true
-1
first boolvar
second boolvar
)"
				);
			}
		}
	}
}

SCENARIO("snapshot failed inside execution _ #111", "[fixes]")
{
	GIVEN("story with multiline output with a knot")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "111_crash.bin")};
		std::unique_ptr<story> ink2{story::from_file(INK_TEST_RESOURCE_DIR "111_crash.bin")};
		runner                 thread = ink->new_runner();
		WHEN("run store and reload")
		{
			auto line = thread->getline();
			THEN("outputs first line") { REQUIRE(line == "First line of text\n"); }
			std::unique_ptr<ink::runtime::snapshot> snapshot{thread->create_snapshot()};
			runner                                  thread2 = ink2->new_runner_from_snapshot(*snapshot);
			line                                            = thread->getline();
			THEN("outputs second line") { REQUIRE(line == "Second line of test\n"); }
		}
	}
}

SCENARIO("missing leading whitespace inside choice-only text and glued text _ #130 #131", "[fixes]")
{
	GIVEN("story with problematic text")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR
		                                            "130_131_missing_whitespace.bin")};
		runner                 thread = ink->new_runner();
		WHEN("run story")
		{
			auto line = thread->getline();
			THEN("expect spaces in glued text") { REQUIRE(line == "Glue with no gaps.\n"); }
			THEN("choice contains space")
			{
				REQUIRE(thread->num_choices() == 1);
				REQUIRE(std::string(thread->get_choice(0)->text()) == "Look around");
			}
			thread->choose(0);
			line = thread->getall();
			THEN("no space in post choice text")
			{
				REQUIRE(line == "Looking around the saloon, you don't find much.");
			}
		}
	}
}

SCENARIO("Casting during redefinition is too strict _ #134", "[fixes]")
{
	GIVEN("story with problematic text")
	{
		auto   ink    = story::from_file(INK_TEST_RESOURCE_DIR "134_restrictive_casts.bin");
		runner thread = ink->new_runner();

		WHEN("run story")
		{
			// Initial casts/assignments are allowed.
			auto line = thread->getline();
			THEN("expect initial values") { REQUIRE(line == "true 1 1 text A\n"); }
			line = thread->getline();
			THEN("expect evaluated") { REQUIRE(line == "1.5 1.5 1.5 text0.5 B\n"); }
			line = thread->getline();
			THEN("expect assigned") { REQUIRE(line == "1.5 1.5 1.5 text0.5 B\n"); }
		}

		// Six cases that should fail. We can't pollute lookahead with these so they need to be
		// separated out.
		for (int i = 0; i < 6; ++i) {
			WHEN("Jump to failing case")
			{
				const std::string name = "Fail" + std::to_string(i);
				REQUIRE_NOTHROW(thread->move_to(ink::hash_string(name.c_str())));
				std::string line;
				REQUIRE_THROWS_AS(line = thread->getline(), ink::ink_exception);
			}
		}
	}
}

SCENARIO("Using knot visit count as condition _ #139", "[fixes]")
{
	GIVEN("story with conditional choice.")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "139_conditional_choice.bin")
		};
		runner                 thread = ink->new_runner();
		WHEN("visit knot 'one' an going back to choice")
		{
			std::string content = thread->getall();
			REQUIRE_FALSE(thread->can_continue());
			REQUIRE(thread->num_choices() == 2);
			thread->choose(1);
			content += thread->getall();
			REQUIRE(content == "Check\nFirst time at one\n");
			THEN("conditinal choice is displayed")
			{
				REQUIRE(thread->num_choices() == 3);
				CHECK(thread->get_choice(0)->text() == std::string("DEFAULT"));
				CHECK(thread->get_choice(1)->text() == std::string("Check"));
				CHECK(thread->get_choice(2)->text() == std::string("Test"));

				WHEN("go to 'one' twice")
				{
					thread->choose(1);
					std::string content = thread->getall();
					REQUIRE(thread->num_choices() == 3);
					THEN("get both one strings") { REQUIRE(content == "Check\nBeen here before\n"); }
				}
			}
		}
		WHEN("loop back to choice")
		{
			std::string content = thread->getall();
			REQUIRE_FALSE(thread->can_continue());
			REQUIRE(thread->num_choices() == 2);
			thread->choose(0);
			content += thread->getall();
			REQUIRE(content == "DEFAULT\nLoopback");
			THEN("conditinal choice is not displayed")
			{
				REQUIRE(thread->num_choices() == 2);
				CHECK(thread->get_choice(0)->text() == std::string("DEFAULT"));
				CHECK(thread->get_choice(1)->text() == std::string("Check"));
			}
		}
	}
}
