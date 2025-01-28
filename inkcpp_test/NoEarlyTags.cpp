#include "catch.hpp"

#include <compiler.h>
#include <globals.h>
#include <runner.h>
#include <story.h>

using namespace ink::runtime;

auto tg_ink    = story::from_file(INK_TEST_RESOURCE_DIR "NoEarlyTags.bin");
auto tg_thread = tg_ink->new_runner();

SCENARIO("Story with tags and glues", "[tags][glue]")
{
	GIVEN("lines intersected with tags and glue")
	{
		WHEN("no glue")
		{
			CHECK(tg_thread->getline() == "Hey there, nice to meet you!\n");
			REQUIRE(tg_thread->num_tags() == 2);
			CHECK(std::string(tg_thread->get_tag(0)) == "name fae03_name");
			CHECK(std::string(tg_thread->get_tag(1)) == "bb CaoimheGenericProgress");
		}
		WHEN("next line")
		{
			CHECK(tg_thread->getline() == "Hey, I'm Hey and this is YOU, nice to meet you too!\n");
			REQUIRE(tg_thread->num_tags() == 1);
			CHECK(std::string(tg_thread->get_tag(0)) == "name fae00_name");
		}
		WHEN("glue stops tags lookahead")
		{
			CHECK(
			    tg_thread->getline() == "I'm Do! Most people can't pronounce it, just think 'Kee-vah\".\n"
			);
			REQUIRE(tg_thread->num_tags() == 2);
			CHECK(std::string(tg_thread->get_tag(0)) == "name fae03_name");
			CHECK(std::string(tg_thread->get_tag(1)) == "meet-character 5");
		}
	}
}
