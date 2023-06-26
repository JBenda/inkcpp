#include "catch.hpp"
#include "../inkcpp_cl/test.h"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>
#include <choice.h>

using namespace ink::runtime;

SCENARIO("tags", "[tags]")
{
	inklecate("ink/AHF.ink", "AHF.tmp");
	ink::compiler::run("AHF.tmp", "AHF.bin");
	auto ink = story::from_file("AHF.bin");
	runner thread = ink->new_runner();
	thread->move_to(ink::hash_string("test_knot"));
	while(thread->can_continue()) {
		auto line = thread->getline();
	}
	REQUIRE(thread->can_continue() == false);
}

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
						itr[0].text(),
						itr[1].text()
					};
					THEN("choices won't print tags, tags are still the same, but they can contain tags")
					{
						REQUIRE(choices[0] == "a");
						REQUIRE(choices[1] == "b");
						REQUIRE(thread->has_tags());
						REQUIRE(thread->num_tags() == 4);
						REQUIRE(std::string(thread->get_tag(0)) == "global_tag");
						REQUIRE(std::string(thread->get_tag(1)) == "knot_tag_start");
						REQUIRE(std::string(thread->get_tag(2)) == "second_knot_tag_start");
						REQUIRE(std::string(thread->get_tag(3)) == "output_tag_h");

						REQUIRE_FALSE(itr[0].has_tags());
						REQUIRE(itr[0].num_tags() == 0);
						REQUIRE(itr[1].has_tags());

						REQUIRE(itr[1].num_tags() == 2);
						REQUIRE(std::string(itr[1].get_tag(0)) == "choice_tag_b");
						REQUIRE(std::string(itr[1].get_tag(1)) == "choice_tag_b_2");
					}
					WHEN("choose divert")
					{
						thread->choose(1);
						THEN("choosing won't add tags!")
						{
							REQUIRE_FALSE(thread->has_tags());
							REQUIRE(thread->num_tags() == 0);
						}
						WHEN("proceed")
						{
							std::string line = thread->getall();
							THEN("new knot tag and now line tag, also choice tag. AND dont print tag in choice")
							{
								REQUIRE(line == "Knot2\n");
								REQUIRE(thread->has_tags());
								REQUIRE(thread->num_tags() == 2);
								REQUIRE(std::string(thread->get_tag(0)) == "knot_tag_2");
								REQUIRE(std::string(thread->get_tag(1)) == "output_tag_k");

								auto itr = thread->begin();
								REQUIRE(std::string(itr[0].text()) == "e");
								REQUIRE(std::string(itr[1].text()) == "f with detail");
								REQUIRE(std::string(itr[2].text()) == "g");

								REQUIRE_FALSE(itr[0].has_tags());
								REQUIRE(itr[0].num_tags() == 0);
								REQUIRE(itr[1].has_tags());
								REQUIRE(itr[1].num_tags() == 4);
								REQUIRE(std::string(itr[1].get_tag(0)) == "shared_tag");
								REQUIRE(std::string(itr[1].get_tag(1)) == "shared_tag_2");
								REQUIRE(std::string(itr[1].get_tag(2)) == "choice_tag");
								REQUIRE(std::string(itr[1].get_tag(3)) == "choice_tag_2");
								REQUIRE(itr[2].has_tags());
								REQUIRE(itr[2].num_tags() == 1);
							}
							WHEN("choose choice with tag, and proceed to end")
							{
								thread->choose(1);
								auto line = thread->getall();

								REQUIRE(line == "f and content\nout");
								REQUIRE(thread->has_tags());
								REQUIRE(thread->num_tags() == 5);
								REQUIRE(std::string(thread->get_tag(0)) == "shared_tag");
								REQUIRE(std::string(thread->get_tag(1)) == "shared_tag_2");
								REQUIRE(std::string(thread->get_tag(2)) == "content_tag");
								REQUIRE(std::string(thread->get_tag(3)) == "content_tag_2");
								REQUIRE(std::string(thread->get_tag(4)) == "close_tag");
							}
						}
					}
				}
			}
		}
	}
}
