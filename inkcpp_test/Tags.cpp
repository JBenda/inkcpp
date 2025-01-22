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
		WHEN("starting the thread")
		{
			CHECK_FALSE(_thread->has_tags());
		}
		WHEN("on the first line")
		{
			CHECK(_thread->getline() == "First line has global tags only\n");
			THEN("it has the global tags")
			{
				CHECK(_thread->has_tags());
				CHECK(_thread->num_tags() == 1);
				CHECK(std::string(_thread->get_tag(0)) == "global_tag");
			}
		}
		WHEN("on the second line")
		{
			CHECK(_thread->getline() == "Second line has one tag\n");
			THEN("it has one tag")
			{
				CHECK(_thread->has_tags());;
				REQUIRE(_thread->num_tags() == 1);
				CHECK(std::string(_thread->get_tag(0)) == "tagged");
			}
		}
		WHEN("on the third line")
		{
			CHECK(_thread->getline() == "Third line has two tags\n");
			THEN("it has two tags")
			{
				CHECK(_thread->has_tags());
				REQUIRE(_thread->num_tags() == 2);
				CHECK(std::string(_thread->get_tag(0)) == "tag next line");
				CHECK(std::string(_thread->get_tag(1)) == "more tags");
			}
		}
		WHEN("on the fourth line")
		{
			CHECK(_thread->getline() == "Fourth line has three tags\n");

			THEN("it has three tags")
			{
				CHECK(_thread->has_tags());
				REQUIRE(_thread->num_tags() == 3);
				CHECK(std::string(_thread->get_tag(0)) == "above");
				CHECK(std::string(_thread->get_tag(1)) == "side");
				CHECK(std::string(_thread->get_tag(2)) == "across");
			}
		}
		WHEN("entering a knot")
		{
			CHECK(_thread->getline() == "Hello\n");
			THEN("it has four tags")
			{
				CHECK(_thread->has_tags());
				REQUIRE(_thread->num_tags() == 4);
				CHECK(std::string(_thread->get_tag(0)) == "knot_tag_start");
				CHECK(std::string(_thread->get_tag(1)) == "second_knot_tag_start");
				CHECK(std::string(_thread->get_tag(2)) == "third_knot_tag");
				CHECK(std::string(_thread->get_tag(3)) == "output_tag_h");
			}
		}
		WHEN("on the next line")
		{
			CHECK(_thread->getline() == "Second line has no tags\n");
			THEN("it has no tags")
			{
				CHECK_FALSE(_thread->has_tags());
				REQUIRE(_thread->num_tags() == 0);
			}
		}
		WHEN("at the first choice list")
		{
			CHECK(_thread->getline() == "");

			REQUIRE(std::distance(_thread->begin(), _thread->end()) == 2);
			auto choice_list = _thread->begin();

			THEN("first choice has no tags")
			{
				CHECK(std::string(choice_list[0].text()) == "a");
				CHECK_FALSE(choice_list[0].has_tags());
				REQUIRE(choice_list[0].num_tags() == 0);
			}
			THEN("second choice has two tags")
			{
				CHECK(std::string(choice_list[1].text()) == "b");
				CHECK(choice_list[1].has_tags());
				REQUIRE(choice_list[1].num_tags() == 2);
				CHECK(std::string(choice_list[1].get_tag(0)) == "choice_tag_b");
				CHECK(std::string(choice_list[1].get_tag(1)) == "choice_tag_b_2");
			}
		}
		WHEN("selecting the second choice")
		{
			_thread->choose(1);

			CHECK(_thread->getline() == "Knot2\n");
			THEN("it has two tags")
			{
				CHECK(_thread->has_tags());
				REQUIRE(_thread->num_tags() == 2);
				CHECK(std::string(_thread->get_tag(0)) == "knot_tag_2");
				CHECK(std::string(_thread->get_tag(1)) == "output_tag_k");
			}
		}
		WHEN("at the second choice list")
		{
			CHECK(_thread->getline() == "");

			REQUIRE(std::distance(_thread->begin(), _thread->end()) == 3);
			auto choice_list = _thread->begin();

			THEN("first choice has no tags")
			{
				CHECK(std::string(choice_list[0].text()) == "e");
				CHECK_FALSE(choice_list[0].has_tags());
				REQUIRE(choice_list[0].num_tags() == 0);
			}
			THEN("second choice has four tags")
			{
				CHECK(std::string(choice_list[1].text()) == "f with detail");
				CHECK(choice_list[1].has_tags());
				REQUIRE(choice_list[1].num_tags() == 4);
				CHECK(std::string(choice_list[1].get_tag(0)) == "shared_tag");
				CHECK(std::string(choice_list[1].get_tag(1)) == "shared_tag_2");
				CHECK(std::string(choice_list[1].get_tag(2)) == "choice_tag");
				CHECK(std::string(choice_list[1].get_tag(3)) == "choice_tag_2");
			}
			THEN("third choice has one tag")
			{
				CHECK(std::string(choice_list[2].text()) == "g");
				CHECK(choice_list[2].has_tags());
				REQUIRE(choice_list[2].num_tags() == 1);
				CHECK(std::string(choice_list[2].get_tag(0)) == "choice_tag_g");
			}
		}
		WHEN("selecting the choice with shared tags")
		{
			_thread->choose(1);

			CHECK(_thread->getline() == "f and content\n");
			THEN("it has four tags")
			{
				CHECK(_thread->has_tags());
				REQUIRE(_thread->num_tags() == 4);
				CHECK(std::string(_thread->get_tag(0)) == "shared_tag");
				CHECK(std::string(_thread->get_tag(1)) == "shared_tag_2");
				CHECK(std::string(_thread->get_tag(2)) == "content_tag");
				CHECK(std::string(_thread->get_tag(3)) == "content_tag_2");
			}
		}
		WHEN("on the last line")
		{
			CHECK(_thread->getline() == "out\n");
			THEN("it has one tag")
			{
				CHECK(_thread->has_tags());
				REQUIRE(_thread->num_tags() == 1);
				CHECK(std::string(_thread->get_tag(0)) == "close_tag");
			}
		}
	}
}
