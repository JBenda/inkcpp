#include "catch.hpp"

#include <../runner_impl.h>
#include <choice.h>
#include <compiler.h>
#include <globals.h>
#include <runner.h>
#include <story.h>

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

// SCENARIO("adding and removing tags", "[tags][interface]")
// {
// 	story* ink = story::from_file(INK_TEST_RESOURCE_DIR "TagsStory.bin");

// 	using tags_level      = internal::runner_impl::tags_level;
// 	using tags_clear_type = internal::runner_impl::tags_clear_type;

// 	GIVEN("an empty thread")
// 	{
// 		auto thread = ink->new_runner().cast<internal::runner_impl>();

// 		WHEN("retrieving tags out of bounds")
// 		{
// 			CHECK(thread->get_global_tag(0) == nullptr);
// 			CHECK(thread->get_global_tag(10) == nullptr);
// 			CHECK(thread->get_tag(0) == nullptr);
// 			CHECK(thread->get_tag(3) == nullptr);
// 		}

// 		WHEN("adding a global tag")
// 		{
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("kittens", tags_level::GLOBAL).ptr())
// 			    == "kittens"
// 			);

// 			CHECK(thread->num_global_tags() == 1);
// 			REQUIRE(std::string(thread->get_global_tag(0)) == "kittens");
// 			REQUIRE(thread->num_tags() == 0);
// 		}

// 		WHEN("adding a choice tag")
// 		{
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("chance", tags_level::CHOICE).ptr())
// 			    == "chance"
// 			);

// 			REQUIRE(thread->num_global_tags() == 0);
// 			REQUIRE(thread->num_tags() == 1);
// 			CHECK(std::string(thread->get_tag(0)) == "chance");
// 		}

// 		WHEN("adding a line tag")
// 		{
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("chicken", tags_level::LINE).ptr())
// 			    == "chicken"
// 			);

// 			REQUIRE(thread->num_global_tags() == 0);
// 			REQUIRE(thread->num_tags() == 1);
// 			CHECK(std::string(thread->get_tag(0)) == "chicken");
// 		}

// 		WHEN("adding two tags of the same type")
// 		{
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("bacon", tags_level::LINE).ptr()) == "bacon"
// 			);
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("cheese", tags_level::LINE).ptr())
// 			    == "cheese"
// 			);

// 			REQUIRE(thread->num_global_tags() == 0);
// 			REQUIRE(thread->num_tags() == 2);
// 			CHECK(std::string(thread->get_tag(0)) == "bacon");
// 			CHECK(std::string(thread->get_tag(1)) == "cheese");
// 		}

// 		WHEN("adding multiple tags in the correct order")
// 		{
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("written by wake", tags_level::GLOBAL).ptr())
// 			    == "written by wake"
// 			);
// 			CHECK(
// 			    std::string(
// 			        ( const char* ) *thread->add_tag("turn on flashlight", tags_level::CHOICE).ptr()
// 			    )
// 			    == "turn on flashlight"
// 			);
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("cover", tags_level::CHOICE).ptr())
// 			    == "cover"
// 			);
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("point at darkness", tags_level::LINE).ptr())
// 			    == "point at darkness"
// 			);

// 			REQUIRE(thread->num_global_tags() == 1);
// 			CHECK(std::string(thread->get_global_tag(0)) == "written by wake");
// 			REQUIRE(thread->num_tags() == 3);
// 			CHECK(std::string(thread->get_tag(0)) == "turn on flashlight");
// 			CHECK(std::string(thread->get_tag(1)) == "cover");
// 			CHECK(std::string(thread->get_tag(2)) == "point at darkness");
// 		}

// 		WHEN("adding multiple tags in the wrong order")
// 		{
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("across", tags_level::LINE).ptr())
// 			    == "across"
// 			);
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("fox", tags_level::CHOICE).ptr()) == "fox"
// 			);
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("the", tags_level::GLOBAL).ptr()) == "the"
// 			);
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("time", tags_level::LINE).ptr()) == "time"
// 			);
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("dashes", tags_level::CHOICE).ptr())
// 			    == "dashes"
// 			);
// 			CHECK(
// 			    std::string(( const char* ) *thread->add_tag("busy", tags_level::GLOBAL).ptr()) == "busy"
// 			);

// 			REQUIRE(thread->num_global_tags() == 2);
// 			CHECK(std::string(thread->get_global_tag(0)) == "the");
// 			CHECK(std::string(thread->get_global_tag(1)) == "busy");
// 			REQUIRE(thread->num_tags() == 4);
// 			CHECK(std::string(thread->get_tag(0)) == "fox");
// 			CHECK(std::string(thread->get_tag(1)) == "dashes");
// 			CHECK(std::string(thread->get_tag(2)) == "across");
// 			CHECK(std::string(thread->get_tag(3)) == "time");
// 		}
// 	}

// 	GIVEN("a thread with tags")
// 	{
// 		auto thread = ink->new_runner().cast<internal::runner_impl>();
// 		thread->add_tag("the", tags_level::GLOBAL);
// 		thread->add_tag("busy", tags_level::GLOBAL);
// 		thread->add_tag("fox", tags_level::CHOICE);
// 		thread->add_tag("dashes", tags_level::CHOICE);
// 		thread->add_tag("across", tags_level::LINE);
// 		thread->add_tag("time", tags_level::LINE);

// 		WHEN("clearing all tags")
// 		{
// 			thread->clear_tags(tags_clear_type::ALL);

// 			CHECK_FALSE(thread->has_tags());
// 			REQUIRE(thread->num_global_tags() == 0);
// 			REQUIRE(thread->num_tags() == 0);
// 		}

// 		WHEN("keeping choice tags when clearing")
// 		{
// 			thread->clear_tags(tags_clear_type::KEEP_CHOICE);

// 			REQUIRE(thread->num_global_tags() == 2);
// 			CHECK(std::string(thread->get_global_tag(0)) == "the");
// 			CHECK(std::string(thread->get_global_tag(1)) == "busy");
// 			REQUIRE(thread->num_tags() == 2);
// 			CHECK(std::string(thread->get_tag(0)) == "fox");
// 			CHECK(std::string(thread->get_tag(1)) == "dashes");
// 		}

// 		WHEN("keeping global tags when clearing")
// 		{
// 			thread->clear_tags(tags_clear_type::KEEP_GLOBALS);

// 			REQUIRE(thread->num_global_tags() == 2);
// 			CHECK(std::string(thread->get_global_tag(0)) == "the");
// 			CHECK(std::string(thread->get_global_tag(1)) == "busy");
// 			REQUIRE(thread->num_tags() == 0);
// 		}

// 		WHEN("adding a tag after clearing all")
// 		{
// 			thread->clear_tags(tags_clear_type::ALL);
// 			thread->add_tag("tracked", tags_level::LINE);

// 			REQUIRE(thread->num_global_tags() == 0);
// 			REQUIRE(thread->num_tags() == 1);
// 			CHECK(std::string(thread->get_tag(0)) == "tracked");
// 		}

// 		WHEN("adding a global tag after keeping choice tags")
// 		{
// 			thread->clear_tags(tags_clear_type::KEEP_CHOICE);
// 			thread->add_tag("handsome", tags_level::GLOBAL);

// 			REQUIRE(thread->num_global_tags() == 3);
// 			CHECK(std::string(thread->get_global_tag(0)) == "the");
// 			CHECK(std::string(thread->get_global_tag(1)) == "busy");
// 			CHECK(std::string(thread->get_global_tag(2)) == "handsome");
// 			REQUIRE(thread->num_tags() == 2);
// 			CHECK(std::string(thread->get_tag(0)) == "fox");
// 			CHECK(std::string(thread->get_tag(1)) == "dashes");
// 		}

// 		WHEN("adding a choice tag after keeping choice tags")
// 		{
// 			thread->clear_tags(tags_clear_type::KEEP_CHOICE);
// 			thread->add_tag("away", tags_level::CHOICE);

// 			REQUIRE(thread->num_global_tags() == 2);
// 			CHECK(std::string(thread->get_global_tag(0)) == "the");
// 			CHECK(std::string(thread->get_global_tag(1)) == "busy");
// 			REQUIRE(thread->num_tags() == 3);
// 			CHECK(std::string(thread->get_tag(0)) == "fox");
// 			CHECK(std::string(thread->get_tag(1)) == "dashes");
// 			CHECK(std::string(thread->get_tag(2)) == "away");
// 		}

// 		WHEN("adding a line tag after keeping choice tags")
// 		{
// 			thread->clear_tags(tags_clear_type::KEEP_CHOICE);
// 			thread->add_tag("forward", tags_level::LINE);

// 			REQUIRE(thread->num_global_tags() == 2);
// 			CHECK(std::string(thread->get_global_tag(0)) == "the");
// 			CHECK(std::string(thread->get_global_tag(1)) == "busy");
// 			REQUIRE(thread->num_tags() == 3);
// 			CHECK(std::string(thread->get_tag(0)) == "fox");
// 			CHECK(std::string(thread->get_tag(1)) == "dashes");
// 			CHECK(std::string(thread->get_tag(2)) == "forward");
// 		}

// 		WHEN("adding a global tag after keeping global tags")
// 		{
// 			thread->clear_tags(tags_clear_type::KEEP_GLOBALS);
// 			thread->add_tag("elk", tags_level::GLOBAL);

// 			REQUIRE(thread->num_global_tags() == 3);
// 			CHECK(std::string(thread->get_global_tag(0)) == "the");
// 			CHECK(std::string(thread->get_global_tag(1)) == "busy");
// 			CHECK(std::string(thread->get_global_tag(2)) == "elk");
// 			REQUIRE(thread->num_tags() == 0);
// 		}

// 		WHEN("adding a choice tag after keeping global tags")
// 		{
// 			thread->clear_tags(tags_clear_type::KEEP_GLOBALS);
// 			thread->add_tag("mouse", tags_level::CHOICE);

// 			REQUIRE(thread->num_global_tags() == 2);
// 			CHECK(std::string(thread->get_global_tag(0)) == "the");
// 			CHECK(std::string(thread->get_global_tag(1)) == "busy");
// 			REQUIRE(thread->num_tags() == 1);
// 			CHECK(std::string(thread->get_tag(0)) == "mouse");
// 		}

// 		WHEN("adding a line tag after keeping global tags")
// 		{
// 			thread->clear_tags(tags_clear_type::KEEP_GLOBALS);
// 			thread->add_tag("driver", tags_level::LINE);

// 			REQUIRE(thread->num_global_tags() == 2);
// 			CHECK(std::string(thread->get_global_tag(0)) == "the");
// 			CHECK(std::string(thread->get_global_tag(1)) == "busy");
// 			REQUIRE(thread->num_tags() == 1);
// 			CHECK(std::string(thread->get_tag(0)) == "driver");
// 		}
// 	}
// }


SCENARIO("run story with tags", "[tags][story]")
{
	GIVEN("a story with tags")
	{
		story* _ink    = story::from_file(INK_TEST_RESOURCE_DIR "TagsStory.bin");
		runner _thread = _ink->new_runner();
		WHEN("starting the thread") { CHECK_FALSE(_thread->has_tags()); }
		WHEN("on the first line")
		{
			CHECK(_thread->getline() == "First line has global tags only\n");
			THEN("it has the global tags")
			{
				CHECK(_thread->has_global_tags());
				CHECK_FALSE(_thread->has_knot_tags());
				CHECK(_thread->has_tags());
				REQUIRE(_thread->num_global_tags() == 1);
				CHECK(std::string(_thread->get_global_tag(0)) == "global_tag");
				REQUIRE(_thread->num_tags() == 1);
				CHECK(std::string(_thread->get_tag(0)) == "global_tag");

			}
		}
		WHEN("on the second line")
		{
			_thread->getline();
			CHECK(_thread->getline() == "Second line has one tag\n");
			THEN("it has one tag")
			{
				CHECK(_thread->has_tags());
				CHECK(_thread->num_global_tags() == 1);
				CHECK(_thread->num_knot_tags() == 0);
				REQUIRE(_thread->num_tags() == 1);
				CHECK(std::string(_thread->get_tag(0)) == "tagged");
			}
		}
		WHEN("on the third line")
		{
			_thread->getline();
			_thread->getline();
			CHECK(_thread->getline() == "Third line has two tags\n");
			THEN("it has two tags")
			{
				CHECK(_thread->has_tags());
				CHECK(_thread->num_global_tags() == 1);
				CHECK(_thread->num_knot_tags() == 0);
				REQUIRE(_thread->num_tags() == 2);
				CHECK(std::string(_thread->get_tag(0)) == "tag next line");
				CHECK(std::string(_thread->get_tag(1)) == "more tags");
			}
		}
		WHEN("on the fourth line")
		{
			_thread->getline();
			_thread->getline();
			_thread->getline();
			CHECK(_thread->getline() == "Fourth line has three tags\n");

			THEN("it has three tags")
			{
				CHECK(_thread->has_tags());
				CHECK(_thread->num_global_tags() == 1);
				CHECK(_thread->num_knot_tags() == 0);
				REQUIRE(_thread->num_tags() == 3);
				CHECK(std::string(_thread->get_tag(0)) == "above");
				CHECK(std::string(_thread->get_tag(1)) == "side");
				CHECK(std::string(_thread->get_tag(2)) == "across");
			}
		}
		WHEN("entering a knot")
		{
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			CHECK(_thread->getline() == "Hello\n");
			THEN("it has four tags")
			{
				CHECK(_thread->has_tags());
				CHECK(_thread->num_global_tags() == 1);
				REQUIRE(_thread->num_tags() == 4);
				CHECK(std::string(_thread->get_tag(0)) == "knot_tag_start");
				CHECK(std::string(_thread->get_tag(1)) == "second_knot_tag_start");
				CHECK(std::string(_thread->get_tag(2)) == "third_knot_tag");
				CHECK(std::string(_thread->get_tag(3)) == "output_tag_h");
				REQUIRE(_thread->num_knot_tags() == 3);
				CHECK(std::string(_thread->get_knot_tag(0)) == "knot_tag_start");
				CHECK(std::string(_thread->get_knot_tag(1)) == "second_knot_tag_start");
				CHECK(std::string(_thread->get_knot_tag(2)) == "third_knot_tag");
			}
		}
		WHEN("on the next line")
		{
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			CHECK(_thread->getline() == "Second line has no tags\n");
			THEN("it has no tags")
			{
				CHECK(_thread->num_global_tags() == 1);
				CHECK(_thread->num_knot_tags() == 3);
				CHECK_FALSE(_thread->has_tags());
				REQUIRE(_thread->num_tags() == 0);
			}
		}
		WHEN("at the first choice list")
		{
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			CHECK_FALSE(_thread->can_continue());

			REQUIRE(std::distance(_thread->begin(), _thread->end()) == 2);
			auto choice_list = _thread->begin();

			THEN("check tags on choices")
			{
				CHECK(_thread->num_global_tags() == 1);
				CHECK(_thread->num_knot_tags() == 3);
				CHECK(std::string(choice_list[0].text()) == "a");
				CHECK_FALSE(choice_list[0].has_tags());
				REQUIRE(choice_list[0].num_tags() == 0);

				CHECK(std::string(choice_list[1].text()) == "b");
				CHECK(choice_list[1].has_tags());
				REQUIRE(choice_list[1].num_tags() == 2);
				CHECK(std::string(choice_list[1].get_tag(0)) == "choice_tag_b");
				CHECK(std::string(choice_list[1].get_tag(1)) == "choice_tag_b_2");
			}
		}
		WHEN("selecting the second choice")
		{
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->choose(1);

			CHECK(_thread->getline() == "Knot2\n");
			THEN("it has two tags")
			{
				CHECK(_thread->num_global_tags() == 1);
				CHECK(_thread->has_tags());
				REQUIRE(_thread->num_tags() == 2);
				CHECK(std::string(_thread->get_tag(0)) == "knot_tag_2");
				CHECK(std::string(_thread->get_tag(1)) == "output_tag_k");
				CHECK(_thread->has_knot_tags());
				REQUIRE(_thread->num_knot_tags() == 1);
				CHECK(std::string(_thread->get_knot_tag(0)) == "knot_tag_2");
			}
		}
		WHEN("at the second choice list")
		{
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->choose(1);
			_thread->getline();
			CHECK(!_thread->can_continue());

			REQUIRE(std::distance(_thread->begin(), _thread->end()) == 3);
			auto choice_list = _thread->begin();

			THEN("check tags on choices")
			{
				CHECK(_thread->num_global_tags() == 1);
				CHECK(_thread->num_knot_tags() == 1);
				CHECK(_thread->num_tags() == 2);
				CHECK(std::string(choice_list[0].text()) == "e");
				CHECK_FALSE(choice_list[0].has_tags());
				REQUIRE(choice_list[0].num_tags() == 0);

				CHECK(std::string(choice_list[1].text()) == "f with detail");
				CHECK(choice_list[1].has_tags());
				REQUIRE(choice_list[1].num_tags() == 4);
				CHECK(std::string(choice_list[1].get_tag(0)) == "shared_tag");
				CHECK(std::string(choice_list[1].get_tag(1)) == "shared_tag_2");
				CHECK(std::string(choice_list[1].get_tag(2)) == "choice_tag");
				CHECK(std::string(choice_list[1].get_tag(3)) == "choice_tag_2");

				CHECK(std::string(choice_list[2].text()) == "g");
				CHECK(choice_list[2].has_tags());
				REQUIRE(choice_list[2].num_tags() == 1);
				CHECK(std::string(choice_list[2].get_tag(0)) == "choice_tag_g");
			}
		}
		WHEN("selecting the choice with shared tags")
		{
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->choose(1);
			_thread->getline();
			_thread->choose(1);

			REQUIRE(_thread->getline() == "f and content\n");
			THEN("it has four tags")
			{
				CHECK(_thread->num_global_tags() == 1);
				CHECK(_thread->num_knot_tags() == 1);
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
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->getline();
			_thread->choose(1);
			_thread->getline();
			_thread->choose(1);
			_thread->getline();
			CHECK(_thread->getline() == "out\n");
			THEN("it has one tag")
			{
				CHECK(_thread->num_global_tags() == 1);
				CHECK(std::string(_thread->get_global_tag(0)) == "global_tag");
				REQUIRE(_thread->num_knot_tags() == 1);
				CHECK(std::string(_thread->get_knot_tag(0)) == "kont_tag_2");
				CHECK(_thread->has_tags());
				REQUIRE(_thread->num_tags() == 1);
				CHECK(std::string(_thread->get_tag(0)) == "close_tag");
			}
		}
	}
}
