#include "catch.hpp"
#include "system.h"

#include <../runner_impl.h>
#include <choice.h>
#include <compiler.h>
#include <globals.h>
#include <runner.h>
#include <story.h>

using namespace ink::runtime;

SCENARIO("tags", "[tags][runtime]")
{
	GIVEN("a story moved to test_knot")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "AHF.bin")};
		runner                 thread = ink->new_runner();
		thread->move_to(ink::hash_string("test_knot"));

		WHEN("all lines are consumed")
		{
			while (thread->can_continue()) {
				thread->getline();
			}

			THEN("the thread cannot continue") { REQUIRE(thread->can_continue() == false); }
		}
	}
}

SCENARIO("run story with tags", "[tags][runtime]")
{
	GIVEN("a story with tags")
	{
		std::unique_ptr<story> ink{story::from_file(INK_TEST_RESOURCE_DIR "TagsStory.bin")};
		runner                 thread = ink->new_runner();

		THEN("initially there are no tags and the runner has not moved to any knot")
		{
			CHECK_FALSE(thread->has_tags());
			CHECK(thread->get_current_knot() == 0);
		}

		WHEN("the first line is read")
		{
			std::string line = thread->getline();

			THEN("the output is correct and only global tags are present")
			{
				REQUIRE(line == "First line has global tags only\n");
				CHECK(thread->get_current_knot() == ink::hash_string("global_tags_only"));
				CHECK(thread->has_global_tags());
				CHECK_FALSE(thread->has_knot_tags());
				CHECK(thread->has_tags());
				REQUIRE(thread->num_global_tags() == 1);
				CHECK(std::string(thread->get_global_tag(0)) == "global_tag");
				REQUIRE(thread->num_tags() == 1);
				CHECK(std::string(thread->get_tag(0)) == "global_tag");
			}
		}

		WHEN("the second line is read")
		{
			// skip line 1
			thread->getline();
			std::string line = thread->getline();

			THEN("the output is correct and one inline tag is present")
			{
				REQUIRE(line == "Second line has one tag\n");
				CHECK(thread->get_current_knot() == ink::hash_string("global_tags_only"));
				CHECK(thread->has_tags());
				CHECK(thread->num_global_tags() == 1);
				CHECK(thread->num_knot_tags() == 0);
				REQUIRE(thread->num_tags() == 1);
				CHECK(std::string(thread->get_tag(0)) == "tagged");
			}
		}

		WHEN("the third line is read")
		{
			// skip lines 1-2
			for (int i = 0; i < 2; ++i) {
				thread->getline();
			}
			std::string line = thread->getline();

			THEN("the output is correct and two inline tags are present")
			{
				REQUIRE(line == "Third line has two tags\n");
				CHECK(thread->get_current_knot() == ink::hash_string("global_tags_only"));
				CHECK(thread->has_tags());
				CHECK(thread->num_global_tags() == 1);
				CHECK(thread->num_knot_tags() == 0);
				REQUIRE(thread->num_tags() == 2);
				CHECK(std::string(thread->get_tag(0)) == "tag next line");
				CHECK(std::string(thread->get_tag(1)) == "more tags");
			}
		}

		WHEN("the fourth line is read")
		{
			// skip lines 1-3
			for (int i = 0; i < 3; ++i) {
				thread->getline();
			}
			std::string line = thread->getline();

			THEN("the output is correct and three inline tags are present")
			{
				REQUIRE(line == "Fourth line has three tags\n");
				CHECK(thread->get_current_knot() == ink::hash_string("global_tags_only"));
				CHECK(thread->has_tags());
				CHECK(thread->num_global_tags() == 1);
				CHECK(thread->num_knot_tags() == 0);
				REQUIRE(thread->num_tags() == 3);
				CHECK(std::string(thread->get_tag(0)) == "above");
				CHECK(std::string(thread->get_tag(1)) == "side");
				CHECK(std::string(thread->get_tag(2)) == "across");
			}
		}

		WHEN("the knot entry line is read")
		{
			// skip lines 1-4
			for (int i = 0; i < 4; ++i) {
				thread->getline();
			}
			std::string line = thread->getline();

			THEN("the output is correct and knot tags are combined with global and inline tags")
			{
				REQUIRE(line == "Hello\n");
				CHECK(thread->get_current_knot() == ink::hash_string("start"));
				CHECK(thread->has_tags());
				CHECK(thread->num_global_tags() == 1);
				REQUIRE(thread->num_tags() == 4);
				CHECK(std::string(thread->get_tag(0)) == "knot_tag_start");
				CHECK(std::string(thread->get_tag(1)) == "second_knot_tag_start");
				CHECK(std::string(thread->get_tag(2)) == "third_knot_tag");
				CHECK(std::string(thread->get_tag(3)) == "output_tag_h");
				REQUIRE(thread->num_knot_tags() == 3);
				CHECK(std::string(thread->get_knot_tag(0)) == "knot_tag_start");
				CHECK(std::string(thread->get_knot_tag(1)) == "second_knot_tag_start");
				CHECK(std::string(thread->get_knot_tag(2)) == "third_knot_tag");
			}
		}

		WHEN("the line after the knot header is read")
		{
			// skip lines 1-5
			for (int i = 0; i < 5; ++i) {
				thread->getline();
			}
			std::string line = thread->getline();

			THEN("the output is correct and no inline tags are present")
			{
				REQUIRE(line == "Second line has no tags\n");
				CHECK(thread->get_current_knot() == ink::hash_string("start"));
				CHECK(thread->num_global_tags() == 1);
				CHECK(thread->num_knot_tags() == 3);
				CHECK_FALSE(thread->has_tags());
				REQUIRE(thread->num_tags() == 0);
			}
		}

		WHEN("all six lines are consumed and the first choice list appears")
		{
			for (int i = 0; i < 6; ++i) {
				thread->getline();
			}
			auto choices = thread->begin();

			THEN("the runner is at a choice point with two correctly tagged options")
			{
				CHECK_FALSE(thread->can_continue());
				CHECK(thread->get_current_knot() == ink::hash_string("start"));
				CHECK(thread->num_global_tags() == 1);
				CHECK(thread->num_knot_tags() == 3);
				REQUIRE(std::distance(thread->begin(), thread->end()) == 2);

				CHECK(std::string(choices[0].text()) == "a");
				CHECK_FALSE(choices[0].has_tags());
				REQUIRE(choices[0].num_tags() == 0);

				CHECK(std::string(choices[1].text()) == "b");
				CHECK(choices[1].has_tags());
				REQUIRE(choices[1].num_tags() == 2);
				CHECK(std::string(choices[1].get_tag(0)) == "choice_tag_b");
				CHECK(std::string(choices[1].get_tag(1)) == "choice_tag_b_2");
			}
		}

		WHEN("choice 'b' is selected at the first choice list")
		{
			for (int i = 0; i < 6; ++i) {
				thread->getline();
			}
			thread->choose(1);
			std::string line = thread->getline();

			THEN("the output is correct and the new knot's tags are reflected")
			{
				REQUIRE(line == "Knot2\n");
				CHECK(thread->get_current_knot() == ink::hash_string("knot2.sub"));
				INFO(thread->get_knot_tag(0));
				CHECK(thread->num_global_tags() == 1);
				CHECK(thread->has_tags());
				REQUIRE(thread->num_tags() == 2);
				CHECK(std::string(thread->get_tag(0)) == "knot_tag_2");
				CHECK(std::string(thread->get_tag(1)) == "output_tag_k");
				CHECK(thread->has_knot_tags());
				REQUIRE(thread->num_knot_tags() == 1);
				CHECK(std::string(thread->get_knot_tag(0)) == "knot_tag_2");
			}
		}

		WHEN("the runner jumps directly to knot2")
		{
			thread->move_to(ink::hash_string("knot2"));
			std::string line = thread->getline();

			THEN("the output is correct and global tags are absent since the global section was skipped")
			{
				REQUIRE(line == "Knot2\n");
				CHECK(thread->get_current_knot() == ink::hash_string("knot2.sub"));
				CHECK(thread->num_global_tags() == 0);
				CHECK(thread->has_tags());
				REQUIRE(thread->num_tags() == 2);
				CHECK(std::string(thread->get_tag(0)) == "knot_tag_2");
				CHECK(std::string(thread->get_tag(1)) == "output_tag_k");
				CHECK(thread->has_knot_tags());
				REQUIRE(thread->num_knot_tags() == 1);
				CHECK(std::string(thread->get_knot_tag(0)) == "knot_tag_2");
			}
		}

		WHEN("choice 'b' is selected and the second choice list appears")
		{
			for (int i = 0; i < 6; ++i) {
				thread->getline();
			}
			thread->choose(1);
			thread->getline(); // consume "Knot2"
			auto choices = thread->begin();

			THEN("three choices are present with correct tags")
			{
				CHECK_FALSE(thread->can_continue());
				CHECK(thread->get_current_knot() == ink::hash_string("knot2.sub"));
				CHECK(thread->num_global_tags() == 1);
				CHECK(thread->num_knot_tags() == 1);
				CHECK(thread->num_tags() == 2);
				REQUIRE(std::distance(thread->begin(), thread->end()) == 3);

				CHECK(std::string(choices[0].text()) == "e");
				CHECK_FALSE(choices[0].has_tags());
				REQUIRE(choices[0].num_tags() == 0);

				CHECK(std::string(choices[1].text()) == "f with detail");
				CHECK(choices[1].has_tags());
				REQUIRE(choices[1].num_tags() == 4);
				CHECK(std::string(choices[1].get_tag(0)) == "shared_tag");
				CHECK(std::string(choices[1].get_tag(1)) == "shared_tag_2");
				CHECK(std::string(choices[1].get_tag(2)) == "choice_tag");
				CHECK(std::string(choices[1].get_tag(3)) == "choice_tag_2");

				CHECK(std::string(choices[2].text()) == "g");
				CHECK(choices[2].has_tags());
				REQUIRE(choices[2].num_tags() == 1);
				CHECK(std::string(choices[2].get_tag(0)) == "choice_tag_g");
			}
		}

		WHEN("choice 'b' then choice 'f with detail' is selected")
		{
			for (int i = 0; i < 6; ++i) {
				thread->getline();
			}
			thread->choose(1);
			thread->getline(); // "Knot2"
			thread->choose(1);
			std::string line = thread->getline();

			THEN("the output is correct and four tags combining shared and content tags are present")
			{
				REQUIRE(line == "f and content\n");
				CHECK(thread->get_current_knot() == ink::hash_string("knot2.sub"));
				CHECK(thread->num_global_tags() == 1);
				CHECK(thread->num_knot_tags() == 1);
				CHECK(thread->has_tags());
				REQUIRE(thread->num_tags() == 4);
				CHECK(std::string(thread->get_tag(0)) == "shared_tag");
				CHECK(std::string(thread->get_tag(1)) == "shared_tag_2");
				CHECK(std::string(thread->get_tag(2)) == "content_tag");
				CHECK(std::string(thread->get_tag(3)) == "content_tag_2");
			}
		}

		WHEN("the story is played through to the final line")
		{
			for (int i = 0; i < 6; ++i) {
				thread->getline();
			}
			thread->choose(1);
			thread->getline(); // "Knot2"
			thread->choose(1);
			thread->getline(); // "f and content"
			std::string line = thread->getline();

			THEN("the output is correct and one closing tag is present alongside global and knot tags")
			{
				REQUIRE(line == "out\n");
				CHECK(thread->get_current_knot() == ink::hash_string("knot2.sub"));
				CHECK(thread->num_global_tags() == 1);
				CHECK(std::string(thread->get_global_tag(0)) == "global_tag");
				REQUIRE(thread->num_knot_tags() == 1);
				CHECK(std::string(thread->get_knot_tag(0)) == "knot_tag_2");
				CHECK(thread->has_tags());
				REQUIRE(thread->num_tags() == 1);
				CHECK(std::string(thread->get_tag(0)) == "close_tag");
			}
		}
	}
}
