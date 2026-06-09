#include "catch.hpp"
#include "../snapshot_impl.h"
#include "list.h"
#include "system.h"

#include <memory>
#include <story.h>
#include <globals.h>
#include <choice.h>
#include <runner.h>
#include <compiler.h>
#include <snapshot.h>
#include <system_error>

using namespace ink::runtime;

SCENARIO("UE example story snapshot migratability", "[snapshot][migration][integration]")
{
	std::unique_ptr<story> base_story{story::from_file(INK_TEST_RESOURCE_DIR "UE_example.bin")};

	GIVEN("a story with shared globals and no active runners")
	{
		globals base_globals = base_story->new_globals();

		THEN("Cannot create a snapshot from a not started globals.")
		{
			REQUIRE_THROWS_MATCHES(
			    base_globals->create_snapshot(), ink::ink_exception,
			    Catch::Message("Only support snapshot of globals with runner! or you don't need a "
			                   "snapshot for this state")
			);
		}

		WHEN("a side runner has finished")
		{
			runner side_thread = base_story->new_runner(base_globals);
			REQUIRE(side_thread->move_to("Wait"));
			REQUIRE(side_thread->getall() == "");

			THEN("a snapshot is migratable")
			{
				std::unique_ptr<snapshot> snap{base_globals->create_snapshot()};
				REQUIRE(snap->can_be_migrated());
			}

			AND_WHEN("a main runner is waiting at a choice")
			{
				runner main_thread = base_story->new_runner(base_globals);
				REQUIRE(
				    main_thread->getall()
				    == "You step outside your car. Its a wired feeling beehing here again.\n"
				);

				THEN("a snapshot is NOT migratable")
				{
					std::unique_ptr<snapshot> snap{base_globals->create_snapshot()};
					REQUIRE_FALSE(snap->can_be_migrated());
				}

				AND_WHEN("the main runner has made a choice")
				{
					main_thread->choose(0);

					THEN("a snapshot is migratable")
					{
						std::unique_ptr<snapshot> snap{base_globals->create_snapshot()};
						REQUIRE(snap->can_be_migrated());
					}
				}
			}
		}
	}
}

SCENARIO("UE example story migration from v1 to v2", "[snapshot][migration][integration]")
{
	std::unique_ptr<story> base_story{story::from_file(INK_TEST_RESOURCE_DIR "UE_example.bin")};
	std::unique_ptr<story> story_v2{story::from_file(INK_TEST_RESOURCE_DIR "UE_example_v2.bin")};

	GIVEN("a story advanced to a migration checkpoint")
	{
		globals base_globals = base_story->new_globals();
		runner  side_thread  = base_story->new_runner(base_globals);
		REQUIRE(side_thread->move_to("Wait"));
		REQUIRE(side_thread->getall() == "");

		runner main_thread = base_story->new_runner(base_globals);
		REQUIRE(
		    main_thread->getall()
		    == "You step outside your car. Its a wired feeling beehing here again.\n"
		);
		main_thread->choose(1);
		main_thread->getall();
		main_thread->choose(1);
		main_thread->getall();
		main_thread->choose(1);

		THEN("the inventory should reflect story progress")
		{
			{
				ink::optional<value> inventory = base_globals->get<value>("Inventory");
				REQUIRE(inventory);
				list                     inventory_list = inventory.value().get<value::Type::List>();
				list_interface::iterator list_iter      = inventory_list->begin();
				REQUIRE(list_iter != inventory_list->end());
				list_interface::iterator::Flag flag = *list_iter;
				REQUIRE(flag.flag_name == std::string("Skull"));
				REQUIRE(flag.list_name == std::string("Clues"));
				++list_iter;
				REQUIRE(list_iter != inventory_list->end());
				flag = *list_iter;
				REQUIRE(flag.flag_name == std::string("TalkWithAnimals"));
				REQUIRE(flag.list_name == std::string("Potions"));
			}
			{
				ink::optional<value> knowladge = base_globals->get<value>("Knowladge");
				REQUIRE(knowladge);
				list                     knowlagde_flag = knowladge.value().get<value::Type::List>();
				list_interface::iterator list_iter      = knowlagde_flag->begin();
				REQUIRE(list_iter != knowlagde_flag->end());
				list_interface::iterator::Flag flag = *list_iter;
				REQUIRE(flag.flag_name == std::string("YellowDress"));
				REQUIRE(flag.list_name == std::string("Knowladge"));
				++list_iter;
				REQUIRE(list_iter == knowlagde_flag->end());
			}
		}

		WHEN("the story globals and runners are migrated to story v2")
		{
			std::unique_ptr<snapshot> snap{base_globals->create_snapshot()};
			globals                   globals_v2 = story_v2->new_globals_from_snapshot(*snap);
			runner main_thread_v2 = story_v2->new_runner_from_snapshot(*snap, globals_v2, 1);

			THEN("the inventory data should be preserved after migration")
			{
				ink::optional<value> inventory = globals_v2->get<value>("Inventory");
				REQUIRE(inventory);
				list                     inventory_list = inventory.value().get<value::Type::List>();
				list_interface::iterator list_iter      = inventory_list->begin();
				REQUIRE(list_iter != inventory_list->end());
				list_interface::iterator::Flag flag = *list_iter;
				REQUIRE(flag.flag_name == std::string("Skull"));
				REQUIRE(flag.list_name == std::string("Clues"));
				++list_iter;
				REQUIRE(list_iter != inventory_list->end());
				flag = *list_iter;
				REQUIRE(flag.flag_name == std::string("TalkWithAnimals"));
				REQUIRE(flag.list_name == std::string("Potions"));
			}

			AND_WHEN("the side thread in v2 drinks the TalkWithAnimals potion")
			{
				runner side_thread_v2 = story_v2->new_runner_from_snapshot(*snap, globals_v2, 0);
				REQUIRE(side_thread_v2->move_to("TPotions.TTalkWithAnimals"));
				REQUIRE(
				    side_thread_v2->getall()
				    == "A potion which allows the consumer to talk with a variaty of animals. Just make sure\nyour serroundings do not think you are crazy.\n"
				);
				side_thread_v2->choose(1);
				REQUIRE(
				    side_thread_v2->getall() == "A take a sip. The potion tastes like Hores, it is afull.\n"
				);
				REQUIRE_FALSE(side_thread_v2->can_continue());
				REQUIRE_FALSE(side_thread_v2->has_choices());

				THEN("the player should be able to talk with animals")
				{
					{
						ink::optional<value> state = globals_v2->get<value>("StatusConditions");
						REQUIRE(state);
						list                     state_list = state.value().get<value::Type::List>();
						list_interface::iterator list_iter  = state_list->begin();
						REQUIRE(list_iter != state_list->end());
						list_interface::iterator::Flag flag = *list_iter;
						REQUIRE(flag.flag_name == std::string("CanTalkWithAniamls"));
						REQUIRE(flag.list_name == std::string("StatusConditions"));
						++list_iter;
						REQUIRE(list_iter == state_list->end());
					}
					{
						ink::optional<value> prototype = globals_v2->get<value>("CanTalkWithAniamls");
						REQUIRE(prototype);
						list                     prototype_flag = prototype.value().get<value::Type::List>();
						list_interface::iterator list_iter      = prototype_flag->begin();
						REQUIRE(list_iter != prototype_flag->end());
						list_interface::iterator::Flag flag = *list_iter;
						REQUIRE(flag.flag_name == std::string("CanTalkWithAniamls"));
						REQUIRE(flag.list_name == std::string("StatusConditions"));
						++list_iter;
						REQUIRE(list_iter == prototype_flag->end());
					}
					{
						ink::optional<value> knowladge = globals_v2->get<value>("Knowladge");
						REQUIRE(knowladge);
						list                     knowlagde_flag = knowladge.value().get<value::Type::List>();
						list_interface::iterator list_iter      = knowlagde_flag->begin();
						REQUIRE(list_iter != knowlagde_flag->end());
						list_interface::iterator::Flag flag = *list_iter;
						REQUIRE(flag.flag_name == std::string("YellowDress"));
						REQUIRE(flag.list_name == std::string("Knowladge"));
						++list_iter;
						REQUIRE(list_iter == knowlagde_flag->end());
					}
				}

				THEN("the main thread continues with mouse dialogue")
				{
					REQUIRE(
					    main_thread_v2->getall()
					    == "\"<Red>Ahh</>\", you cry while reaching for the door bell. Saying it was charched would be an understatement.\n"
					);
					REQUIRE(main_thread_v2->num_choices() == 3);
					REQUIRE(main_thread_v2->get_choice(0)->text() == std::string("look around"));
					REQUIRE(main_thread_v2->get_choice(1)->text() == std::string("Knock again?"));
					REQUIRE(main_thread_v2->get_choice(2)->text() == std::string("Inspect the Door"));
					main_thread_v2->choose(2);
					REQUIRE(
					    main_thread_v2->getall()
					    == "You just saw someone enter, how did they do not get shoked?\nSomething hushes through a hole beside the door, after you come closer you see it. A little gray mouse, it looks quite eloquent.\n"
					);
					main_thread_v2->choose(0);
					REQUIRE(
					    main_thread_v2->getall()
					    == "You try to formulate your dilemma and your annoyance about the doorbell.\n(enter nice conversasion with a picky but helpful mouse)\n"
					);
					REQUIRE(main_thread_v2->num_choices() == 4);
					REQUIRE(main_thread_v2->get_choice(0)->text() == std::string("look around"));
					REQUIRE(main_thread_v2->get_choice(1)->text() == std::string("Knock again?"));
					REQUIRE(main_thread_v2->get_choice(2)->text() == std::string("Inspect the Door"));
					REQUIRE(main_thread_v2->get_choice(3)->text() == std::string("Open the Door"));
				}
			}
		}
	}
}
