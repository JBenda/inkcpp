#include "catch.hpp"
#include "../inkcpp_cl/test.h"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>
#include <choice.h>

using namespace ink::runtime;

SCENARIO("run story with tags", "[tags]")
{
	GIVEN("a story with tags")
	{
		inklecate("ink/TagsStory.ink", "TagsStory.tmp");
		ink::compiler::run("TagsStory.tmp", "TagsStory.bin");
		auto ink = story::from_file("TagsStory.bin");
		runner thread = ink->new_runner();
		WHEN("start thread")
		{
			THEN("No tags")
			{
				REQUIRE(thread->has_tags() == false);
			}
			WHEN("approach first line")
			{
				std::string line = thread->getall();
				THEN("print no tags")
				{
					REQUIRE(line == "Hello\n");
				}
				THEN("collect all previous Tags (global, knot, line) in correct order")
				{
					REQUIRE(thread->has_tags() == true);
					REQUIRE(thread->num_tags() == 4);
					REQUIRE(std::string(thread->get_tag(0)) == "global_tag");
					REQUIRE(std::string(thread->get_tag(1)) == "knot_tag_start");
					REQUIRE(std::string(thread->get_tag(2)) == "second_knot_tag_start");
					REQUIRE(std::string(thread->get_tag(3)) == "output_tag_h");
				}
				WHEN("print choices")
				{
					auto itr = thread->begin();
					std::string choices[2] = {
						(itr++)->text(),
						(itr++)->text()
					};
					THEN("choices won't print tags, tags are still the same")
					{
						REQUIRE(choices[0] == "a");
						REQUIRE(choices[1] == "b");
						REQUIRE(thread->has_tags() == true);
						REQUIRE(thread->num_tags() == 4);
						REQUIRE(std::string(thread->get_tag(0)) == "global_tag");
						REQUIRE(std::string(thread->get_tag(1)) == "knot_tag_start");
						REQUIRE(std::string(thread->get_tag(2)) == "second_knot_tag_start");
						REQUIRE(std::string(thread->get_tag(3)) == "output_tag_h");
					}
					WHEN("choose divert")
					{
						thread->choose(1);
						THEN("choosing won't add tags!")
						{
							REQUIRE(thread->has_tags() == false);
							REQUIRE(thread->num_tags() == 0);
						}
						WHEN("proceed")
						{
							std::string line = thread->getall();
							THEN("new knot tag and now line tag, also choice tag. AND dont print tag in choice")
							{
								REQUIRE(line == "bKnot2\n");
								REQUIRE(thread->has_tags() == true);
								REQUIRE(thread->num_tags() == 3);
								REQUIRE(std::string(thread->get_tag(0)) == "choice_tag_b");
								REQUIRE(std::string(thread->get_tag(1)) == "knot_tag_2");
								REQUIRE(std::string(thread->get_tag(2)) == "output_tag_k");
							}
							WHEN("choose choice without tag, and proceed to end")
							{
								thread->choose(0);
								thread->getall();
								THEN("no tags, tags behind END are ignored")
								{
									REQUIRE(thread->has_tags() == false);
									REQUIRE(thread->num_tags() == 0);
								}
							}
						}
					}
				}
			}
		}
	}
}
