#include "catch.hpp"

#include <compiler.h>
#include <globals.h>
#include <runner.h>
#include <story.h>

using namespace ink::runtime;

SCENARIO("Story with tags and glues", "[tags][glue][runtime]")
{
	GIVEN("lines intersected with tags and glue")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "NoEarlyTags.bin")};
		auto                   thread = ink->new_runner();

		WHEN("the first line is read")
		{
			std::string line = thread->getline();

			THEN("the output is correct and two tags are present")
			{
				CHECK(line == "Hey there, nice to meet you!\n");
				REQUIRE(thread->num_tags() == 2);
				CHECK(std::string(thread->get_tag(0)) == "name fae03_name");
				CHECK(std::string(thread->get_tag(1)) == "bb CaoimheGenericProgress");
			}
		}

		WHEN("the second line is read")
		{
			thread->getline(); // skip line 1
			std::string line = thread->getline();

			THEN("the output is correct and one tag is present")
			{
				CHECK(line == "Hey, I'm Hey and this is YOU, nice to meet you too!\n");
				REQUIRE(thread->num_tags() == 1);
				CHECK(std::string(thread->get_tag(0)) == "name fae00_name");
			}
		}

		WHEN("the third line is read")
		{
			thread->getline(); // skip line 1
			thread->getline(); // skip line 2
			std::string line = thread->getline();

			THEN("glue stops tags lookahead and two tags are present")
			{
				CHECK(line == "I'm Do! Most people can't pronounce it, just think 'Kee-vah\".\n");
				REQUIRE(thread->num_tags() == 2);
				CHECK(std::string(thread->get_tag(0)) == "name fae03_name");
				CHECK(std::string(thread->get_tag(1)) == "meet-character 5");
			}
		}
	}
}
