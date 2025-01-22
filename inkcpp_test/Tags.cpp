#include "catch.hpp"

#include <story.h>
#include <globals.h>
#include <runner.h>
#include <compiler.h>
#include <choice.h>

using namespace ink::runtime;

SCENARIO("tags", "[ahf]")
{
	auto   ink    = story::from_file(INK_TEST_RESOURCE_DIR "AHF.bin");
	runner thread = ink->new_runner();
	thread->move_to(ink::hash_string("test_knot"));
	while(thread->can_continue()) {
		auto line = thread->getline();
	}
	REQUIRE(thread->can_continue() == false);
}

story* _ink = story::from_file(INK_TEST_RESOURCE_DIR "TagsStory.bin");
runner _thread = _ink->new_runner();

SCENARIO("run story with tags", "[tags]")
{
	GIVEN("a story with tags")
	{
		WHEN("start thread")
		{
			THEN("no tags")
			{
				REQUIRE(_thread->has_tags() == false);
			}
		}
		WHEN("first line")
		{
			std::string line = _thread->getline();
			THEN("global tags only")
			{
				REQUIRE(line == "First line has global tags only\n");
				REQUIRE(_thread->has_tags() == true);
				REQUIRE(_thread->num_tags() == 1);
				REQUIRE(std::string(_thread->get_tag(0)) == "global_tag");
			}
		}
		WHEN("second line")
		{
			std::string line = _thread->getline();
			THEN("one tag")
			{
				REQUIRE(line == "Second line has one tag\n");
				REQUIRE(_thread->has_tags() == true);
				REQUIRE(_thread->num_tags() == 1);
				REQUIRE(std::string(_thread->get_tag(0)) == "tagged");
			}
		}
		WHEN("third line")
		{
			std::string line = _thread->getline();
			THEN("two tags")
			{
				REQUIRE(line == "Third line has two tags\n");
				REQUIRE(_thread->has_tags() == true);
				REQUIRE(_thread->num_tags() == 2);
				REQUIRE(std::string(_thread->get_tag(0)) == "tag next line");
				REQUIRE(std::string(_thread->get_tag(1)) == "more tags");
			}
		}
		WHEN("fourth line")
		{
			std::string line = _thread->getline();
			THEN("three tags")
			{
				REQUIRE(line == "Fourth line has three tags\n");
				REQUIRE(_thread->has_tags() == true);
				REQUIRE(_thread->num_tags() == 3);
				REQUIRE(std::string(_thread->get_tag(0)) == "above");
				REQUIRE(std::string(_thread->get_tag(1)) == "side");
				REQUIRE(std::string(_thread->get_tag(2)) == "across");
			}
		}
		WHEN("print choices")
		{
			_thread->getall();
			auto itr = _thread->begin();
			std::string choices[2] = {itr[0].text(), itr[1].text()};
			THEN("choices won't print tags, tags are still the same, but they can contain tags")
			{
				REQUIRE(choices[0] == "a");
				REQUIRE(choices[1] == "b");
				REQUIRE(_thread->has_tags());
				REQUIRE(_thread->num_tags() == 4);
				REQUIRE(std::string(_thread->get_tag(0)) == "global_tag");
				REQUIRE(std::string(_thread->get_tag(1)) == "knot_tag_start");
				REQUIRE(std::string(_thread->get_tag(2)) == "second_knot_tag_start");
				REQUIRE(std::string(_thread->get_tag(3)) == "output_tag_h");

				REQUIRE_FALSE(itr[0].has_tags());
				REQUIRE(itr[0].num_tags() == 0);
				REQUIRE(itr[1].has_tags());

				REQUIRE(itr[1].num_tags() == 2);
				REQUIRE(std::string(itr[1].get_tag(0)) == "choice_tag_b");
				REQUIRE(std::string(itr[1].get_tag(1)) == "choice_tag_b_2");
			}
		}
		WHEN("choose divert")
		{
			_thread->choose(1);
			THEN("choosing won't add tags!")
			{
				REQUIRE_FALSE(_thread->has_tags());
				REQUIRE(_thread->num_tags() == 0);
			}
		}
		WHEN("proceed")
		{
			std::string line = _thread->getall();
			THEN("new knot tag and now line tag, also choice tag. AND dont print tag in choice")
			{
				REQUIRE(line == "Knot2\n");
				REQUIRE(_thread->has_tags());
				REQUIRE(_thread->num_tags() == 2);
				REQUIRE(std::string(_thread->get_tag(0)) == "knot_tag_2");
				REQUIRE(std::string(_thread->get_tag(1)) == "output_tag_k");

				auto itr = _thread->begin();
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
		}
		WHEN("choose choice with tag, and proceed to end")
		{
			_thread->choose(1);
			auto line = _thread->getall();

			REQUIRE(line == "f and content\nout");
			REQUIRE(_thread->has_tags());
			REQUIRE(_thread->num_tags() == 5);
			REQUIRE(std::string(_thread->get_tag(0)) == "shared_tag");
			REQUIRE(std::string(_thread->get_tag(1)) == "shared_tag_2");
			REQUIRE(std::string(_thread->get_tag(2)) == "content_tag");
			REQUIRE(std::string(_thread->get_tag(3)) == "content_tag_2");
			REQUIRE(std::string(_thread->get_tag(4)) == "close_tag");
		}
	}
}
