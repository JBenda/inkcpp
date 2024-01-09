#include "catch.hpp"

#include <compiler.h>
#include <globals.h>
#include <runner.h>
#include <story.h>

using namespace ink::runtime;

SCENARIO("Story with tags and glues", "[glue, tags]")
{
	GIVEN("lines intersected with tags and glue")
	{
		auto ink    = story::from_file(INK_TEST_RESOURCE_DIR "NoEarlyTags.bin");
		auto thread = ink->new_runner();
		WHEN("no glue")
		{
			std::string out = thread->getline();
			REQUIRE(out == "Hey there, nice to meet you!\n");
			REQUIRE(thread->num_tags() == 2);
		}
		WHEN("glue: tags will stop lookahead")
		{
			thread->getline();
			std::string out = thread->getline();
			REQUIRE(out == "Hey, I'm Hey and this is YOU, nice to meet you too!\n");
			REQUIRE(thread->num_tags() == 3);
			out = thread->getline();
			REQUIRE(out == "I'm Do! Most people can't pronounce it, just think 'Kee-vah\".\n");
			REQUIRE(thread->num_tags() == 5);
		}
	}
}
