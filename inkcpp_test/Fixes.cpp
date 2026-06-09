#include "catch.hpp"
#include "snapshot.h"
#include "../snapshot_impl.h"

#include <memory>
#include <story.h>
#include <globals.h>
#include <choice.h>
#include <runner.h>
#include <compiler.h>

#include <sstream>

using namespace ink::runtime;

SCENARIO("string_table fill up #97", "[regression][runtime]")
{
	GIVEN("story murder_scene")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "murder_scene.bin")};
		globals                globStore = ink->new_globals();
		runner                 main      = ink->new_runner(globStore);

		WHEN("the first choice is repeatedly selected until the story ends")
		{
			main->getall();
			while (main->has_choices()) {
				main->choose(0);
				main->getall();
			}

			THEN("the string table has enough room to hold all output without overflow")
			{
				// TEST string table size
				REQUIRE(main->getall().length() == 0);
			}
		}
	}
}

SCENARIO("unknown command _ #109", "[regression][compiler]")
{
	GIVEN("an inline ink JSON story with a boolean variable and conditional branches")
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

		WHEN("the story is compiled and run")
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
			std::unique_ptr<story> ink{story::from_binary(data, static_cast<ink::size_t>(out_str.size()))
			};
			globals                globStore = ink->new_globals();
			runner                 main      = ink->new_runner(globStore);
			std::string            story     = main->getall();

			THEN("compilation produces no warnings or errors and the output is correct")
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

SCENARIO("snapshot failed inside execution _ #111", "[regression][snapshot][runtime]")
{
	GIVEN("a story with multiline output and a knot")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "111_crash.bin")};
		std::unique_ptr<story> ink2{story::from_file(INK_TEST_RESOURCE_DIR "111_crash.bin")};
		runner                 thread = ink->new_runner();

		WHEN("the first line is read")
		{
			std::string line = thread->getline();

			THEN("the first line is output correctly") { REQUIRE(line == "First line of text\n"); }

			AND_WHEN("a snapshot is taken and loaded into a second runner, then the second line is read")
			{
				std::unique_ptr<ink::runtime::snapshot> snapshot{thread->create_snapshot()};
				ink2->new_runner_from_snapshot(*snapshot); // load snapshot into ink2
				std::string line2 = thread->getline();

				THEN("the second line is output correctly") { REQUIRE(line2 == "Second line of test\n"); }
			}
		}
	}
}

SCENARIO(
    "missing leading whitespace inside choice-only text and glued text _ #130 #131",
    "[regression][output][runtime]"
)
{
	GIVEN("a story with problematic whitespace in choices and glued text")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR
		                                            "130_131_missing_whitespace.bin")};
		runner                 thread = ink->new_runner();

		WHEN("the first line is read")
		{
			std::string line = thread->getline();

			THEN("glued text contains the expected spaces") { REQUIRE(line == "Glue with no gaps.\n"); }

			THEN("the choice text contains the expected leading space")
			{
				REQUIRE(thread->num_choices() == 1);
				REQUIRE(std::string(thread->get_choice(0)->text()) == "Look around");
			}

			AND_WHEN("the choice is selected")
			{
				thread->choose(0);
				std::string line2 = thread->getall();

				THEN("post-choice text has no spurious leading space")
				{
					REQUIRE(line2 == "Looking around the saloon, you don't find much.");
				}
			}
		}
	}
}

SCENARIO(
    "choice tag references are not correctly stored (as pointer instead of index) _ #116",
    "[regression][tags][snapshot][runtime]"
)
{
	GIVEN("a story with a choice that has a tag")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR
		                                            "116_story_with_choice_tags.bin")};
		runner                 thread = ink->new_runner();

		WHEN("the story runs to a choice point")
		{
			thread->getall();

			THEN("the choice and its tag are accessible")
			{
				REQUIRE(thread->num_choices() == 1);
				REQUIRE(thread->get_choice(0)->num_tags() == 1);
				REQUIRE(thread->get_choice(0)->get_tag(0) == std::string("Type:Idle"));
			}

			AND_WHEN("a snapshot is taken and loaded into a new runner")
			{
				std::unique_ptr<snapshot> snap{thread->create_snapshot()};
				runner                    loaded = ink->new_runner_from_snapshot(*snap);
				loaded->getall();

				THEN("the choice and its tag are still accessible in the restored runner")
				{
					REQUIRE(loaded->num_choices() == 1);
					REQUIRE(loaded->get_choice(0)->num_tags() == 1);
					REQUIRE(loaded->get_choice(0)->get_tag(0) == std::string("Type:Idle"));
				}
			}

			AND_WHEN("the snapshot is loaded a second time")
			{
				std::unique_ptr<snapshot> snap{thread->create_snapshot()};
				runner                    thread2 = ink->new_runner_from_snapshot(*snap);
				const size_t s = reinterpret_cast<internal::snapshot_impl*>(snap.get())->strings().size();
				ink->new_runner_from_snapshot(*snap);

				THEN("loading the snapshot again does not grow the string table")
				{
					const size_t s2
					    = reinterpret_cast<internal::snapshot_impl*>(snap.get())->strings().size();
					REQUIRE(s == s2);
				}
			}
		}
	}
}

SCENARIO("Casting during redefinition is too strict _ #134", "[regression][runtime]")
{
	GIVEN("a story with mixed-type variable reassignments")
	{
		auto   ink    = story::from_file(INK_TEST_RESOURCE_DIR "134_restrictive_casts.bin");
		runner thread = ink->new_runner();

		WHEN("the first line is read")
		{
			std::string line = thread->getline();

			THEN("initial values are output correctly") { REQUIRE(line == "true 1 1 text A\n"); }
		}

		WHEN("the second line is read")
		{
			thread->getline(); // skip line 1
			std::string line = thread->getline();

			THEN("evaluated values are output correctly") { REQUIRE(line == "1.5 1.5 1.5 text0.5 B\n"); }
		}

		WHEN("the third line is read")
		{
			thread->getline();
			thread->getline(); // skip lines 1-2
			std::string line = thread->getline();

			THEN("assigned values are output correctly") { REQUIRE(line == "1.5 1.5 1.5 text0.5 B\n"); }
		}

		// Six cases that should fail. We can't pollute lookahead with these so they need to be
		// separated out.
		for (int i = 0; i < 6; ++i) {
			WHEN("jumping to a failing cast case")
			{
				const std::string name = "Fail" + std::to_string(i);
				REQUIRE_NOTHROW(thread->move_to(ink::hash_string(name.c_str())));

				THEN("running the story throws an exception for the invalid cast")
				{
					std::string line;
					REQUIRE_THROWS_AS(line = thread->getline(), ink::ink_exception);
				}
			}
		}
	}
}

SCENARIO("Using knot visit count as condition _ #139", "[regression][choices][runtime]")
{
	GIVEN("a story with a conditional choice based on knot visit count")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "139_conditional_choice.bin")
		};
		runner                 thread = ink->new_runner();

		WHEN("knot 'one' is visited via the 'Check' choice")
		{
			std::string content = thread->getall();
			REQUIRE(thread->num_choices() == 2);
			thread->choose(1); // "Check"
			content += thread->getall();
			REQUIRE(content == "Check\nFirst time at one\n");

			THEN("the conditional choice becomes visible after visiting knot 'one'")
			{
				REQUIRE(thread->num_choices() == 3);
				CHECK(thread->get_choice(0)->text() == std::string("DEFAULT"));
				CHECK(thread->get_choice(1)->text() == std::string("Check"));
				CHECK(thread->get_choice(2)->text() == std::string("Test"));
			}

			AND_WHEN("knot 'one' is visited a second time")
			{
				thread->choose(1); // "Check" again
				std::string content2 = thread->getall();

				THEN("both visit strings for knot 'one' are shown")
				{
					REQUIRE(thread->num_choices() == 3);
					REQUIRE(content2 == "Check\nBeen here before\n");
				}
			}
		}

		WHEN("the 'DEFAULT' choice loops back without visiting knot 'one'")
		{
			std::string content = thread->getall();
			REQUIRE(thread->num_choices() == 2);
			thread->choose(0); // "DEFAULT"
			content += thread->getall();
			REQUIRE(content == "DEFAULT\nLoopback");

			THEN("the conditional choice is not displayed")
			{
				REQUIRE(thread->num_choices() == 2);
				CHECK(thread->get_choice(0)->text() == std::string("DEFAULT"));
				CHECK(thread->get_choice(1)->text() == std::string("Check"));
			}
		}
	}
}

SCENARIO("Provoke thread array expension _ #142", "[regression][runtime]")
{
	GIVEN("a story with 15 threads in one knot")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "142_many_threads.bin")};
		runner                 thread = ink->new_runner();

		WHEN("the story runs to the first choice point")
		{
			std::string content = thread->getall();
			REQUIRE(content == "At the top\n");

			THEN("all 15 choices are presented with the correct labels")
			{
				REQUIRE(thread->num_choices() == 15);
				const char options[] = "abcdefghijklmno";
				for (const char* c = options; *c; ++c) {
					CHECK(thread->get_choice(static_cast<ink::size_t>(c - options))->text()[0] == *c);
				}
			}
		}

		WHEN("5 choices are selected in order")
		{
			std::string content = thread->getall();
			for (int i = 0; i < 5; ++i) {
				REQUIRE_FALSE(thread->can_continue());
				thread->choose(i);
				content += thread->getall();
			}
			REQUIRE(
			    content
			    == "At the top\na\nAt the top\nc\nAt the top\ne\nAt the top\ng\nAt the top\ni\nAt the "
			       "top\n"
			);

			THEN("the chosen options are removed and 10 choices remain with the correct labels")
			{
				REQUIRE(thread->num_choices() == 10);
				const char* options = "bdfhjklmno";
				for (const char* c = options; *c; ++c) {
					CHECK(thread->get_choice(static_cast<ink::size_t>(c - options))->text()[0] == *c);
				}
			}
		}
	}
}

SCENARIO("Node text lookup error after loading story", "[regression][runtime][migration]")
{
	GIVEN("UE_example.ink and UE_example2.ink")
	{
		std::unique_ptr<story> v1{story::from_file(INK_TEST_RESOURCE_DIR "UE_example.bin")};
		std::unique_ptr<story> v2{story::from_file(INK_TEST_RESOURCE_DIR "UE_example_v2.bin")};
		WHEN("choosing a choice and v1, loading it in v2 and snap again")
		{
			runner thread = v1->new_runner();
			thread->getall();
			thread->choose(1);
			std::unique_ptr<snapshot> snap{thread->create_snapshot()};
			runner                    thread_v2 = v2->new_runner_from_snapshot(*snap);
			THEN("no assert when snapping again")
			{
				std::unique_ptr<snapshot> snap2{thread_v2->create_snapshot()};
			}
		}
	}
}
