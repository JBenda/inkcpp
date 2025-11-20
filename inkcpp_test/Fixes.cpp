#include "catch.hpp"
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
		auto    ink       = story::from_file(INK_TEST_RESOURCE_DIR "murder_scene.bin");
		globals globStore = ink->new_globals();
		runner  main      = ink->new_runner(globStore);

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
			auto        ink       = story::from_binary(data, out_str.size());
			globals     globStore = ink->new_globals();
			runner      main      = ink->new_runner(globStore);
			std::string story     = main->getall();
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
		auto   ink    = story::from_file(INK_TEST_RESOURCE_DIR "111_crash.bin");
		auto   ink2   = story::from_file(INK_TEST_RESOURCE_DIR "111_crash.bin");
		runner thread = ink->new_runner();
		WHEN("run store and reload")
		{
			auto line = thread->getline();
			THEN("outputs first line") { REQUIRE(line == "First line of text\n"); }
			auto   snapshot = thread->create_snapshot();
			runner thread2  = ink2->new_runner_from_snapshot(*snapshot);
			line            = thread->getline();
			THEN("outputs second line") { REQUIRE(line == "Second line of test\n"); }
		}
	}
}

SCENARIO("missing leading whitespace inside choice-only text and glued text _ #130 #131", "[fixes]")
{
	GIVEN("story with problematic text")
	{
		auto   ink    = story::from_file(INK_TEST_RESOURCE_DIR "130_131_missing_whitespace.bin");
		runner thread = ink->new_runner();
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
