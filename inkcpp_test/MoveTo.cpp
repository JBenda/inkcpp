#include "catch.hpp"
#include "system.h"

#include <story.h>
#include <globals.h>
#include <choice.h>
#include <runner.h>

using namespace ink::runtime;

SCENARIO("run a story, but jump around manually", "[move_to]")
{
	GIVEN("a story with side talking points")
	{
		auto    ink       = story::from_file(INK_TEST_RESOURCE_DIR "MoveTo.bin");
		globals globStore = ink->new_globals();
		runner  main      = ink->new_runner(globStore);
		runner  side      = ink->new_runner(globStore);

		WHEN("just run main story")
		{
			THEN("expect normal output")
			{
				REQUIRE(
				    main->getall()
				    == "Lava kadaver a very boring introduction\nYou are head to head to the minister\n"
				);
				REQUIRE(main->num_choices() == 1);
				REQUIRE(main->get_choice(0)->text() == std::string("Hellow mister menistery"));
				main->choose(0);
				REQUIRE(main->getall() == "Hellow mister menistery\nYou are head to head to the minister\nIt seems you are out of options, you shold give up\n");
			}
		}

		WHEN("skip intorduction")
		{
			main->move_to(ink::hash_string("Dialog.core"));
			THEN("expect output minus introduction")
			{
				REQUIRE(main->getall() == "You are head to head to the minister\n");
				REQUIRE(main->num_choices() == 1);
				REQUIRE(main->get_choice(0)->text() == std::string("Hellow mister menistery"));
				main->choose(0);
				REQUIRE(main->getall() == "Hellow mister menistery\nYou are head to head to the minister\nIt seems you are out of options, you shold give up\n");
			}
		}

		WHEN("second runner is executed")
		{
			main->move_to(ink::hash_string("Dialog.core"));
			side->move_to(ink::hash_string("Transformations.Nothing"));
			THEN("normal main output, and side output")
			{
				REQUIRE(side->getall() == "Hard you could transform if you would like\n");

				REQUIRE(main->getall() == "You are head to head to the minister\n");
				REQUIRE(main->num_choices() == 1);
				REQUIRE(main->get_choice(0)->text() == std::string("Hellow mister menistery"));
				main->choose(0);
				REQUIRE(main->getall() == "Hellow mister menistery\nYou are head to head to the minister\nIt seems you are out of options, you shold give up\n");
			}
		}
		WHEN("second runner modifies value")
		{
			main->move_to(ink::hash_string("Dialog.core"));
			side->move_to(ink::hash_string("Transformations.ToTiger"));
			THEN("main thread output changes")
			{
				REQUIRE(side->getall()  == "Your body growths, and growth, you feel the muscels building.\nAnd there you are, a tiger.\n");
				REQUIRE(main->getall() == "You are head to head to the minister\n");
				REQUIRE(main->num_choices() == 1);
				REQUIRE(main->get_choice(0)->text() == std::string("Roar"));
				main->choose(0);
				REQUIRE(main->getall() == "Grrrrh, Roarrr\nThe people are quit confused\nYou are head to head to the minister\nIt seems you are out of options, you shold give up\n");
			}
		}
		WHEN("execute mutliple small runners with sideeffects")
		{
			main->move_to(ink::hash_string("Dialog.core"));
			side->move_to(ink::hash_string("Transformations.ToTiger"));
			runner side2 = ink->new_runner(globStore);
			side2->move_to(ink::hash_string("Transformations.ToOwl"));
			THEN("side threads should influence each other")
			{
				REQUIRE(side2->getall() == "You Shrink and you get feathers. And, now you are a Owl\n");
				REQUIRE(side->getall()  == "Your body growths, and growth, you feel the muscels building.\nAnd there you are, a tiger.\nyou puke a few feathers, where are they coming from?\n");
				REQUIRE(main->getall() == "You are head to head to the minister\n");
				REQUIRE(main->num_choices() == 1);
				REQUIRE(main->get_choice(0)->text() == std::string("Roar"));
				main->choose(0);
				REQUIRE(main->getall() == "Grrrrh, Roarrr\nThe people are quit confused\nYou are head to head to the minister\nIt seems you are out of options, you shold give up\n");
			}
		}
	}
}
