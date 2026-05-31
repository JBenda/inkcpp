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

using namespace ink::runtime;

SCENARIO("UE example story with multiple runner")
{
	std::unique_ptr<story> base_story{story::from_file(INK_TEST_RESOURCE_DIR "UE_example.bin")};
	std::unique_ptr<story> story_v2{story::from_file(INK_TEST_RESOURCE_DIR "UE_example_v2.bin")};
	globals                base_globals = base_story->new_globals();
	runner                 side_thread  = base_story->new_runner(base_globals);
	GIVEN("a not starde story should be migratable")
	{
		std::unique_ptr<snapshot> snap{base_globals->create_snapshot()};
		REQUIRE(snap->can_be_migrated());
	}
	REQUIRE(side_thread->move_to("Wait"));
	REQUIRE(side_thread->getall() == "");
	GIVEN("a snapshot of a done runner should be migratable")
	{
		std::unique_ptr<snapshot> snap{base_globals->create_snapshot()};
		REQUIRE(snap->can_be_migrated());
	}
	runner main_thread = base_story->new_runner(base_globals);
	REQUIRE(
	    main_thread->getall()
	    == "You step outside your car. Its a wired feeling beehing here again.\n"
	);
	GIVEN("two runner one terminate one in choice should not be migratble")
	{
		std::unique_ptr<snapshot> snap{base_globals->create_snapshot()};
		REQUIRE_FALSE(snap->can_be_migrated());
	}
	GIVEN("two runner, one done the other behind choice should be migratable")
	{
		main_thread->choose(0);
		std::unique_ptr<snapshot> snap{base_globals->create_snapshot()};
		REQUIRE(snap->can_be_migrated());
	}
	GIVEN("Migration")
	{
		main_thread->choose(1);
		main_thread->getall();
		main_thread->choose(1);
		main_thread->getall();
		main_thread->choose(1);
		THEN("Inventory should be as expected")
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

		std::unique_ptr<snapshot> snap{base_globals->create_snapshot()};
		globals                   globals_v2 = story_v2->new_globals_from_snapshot(*snap);
		runner main_thread_v2                = story_v2->new_runner_from_snapshot(*snap, globals_v2, 1);

		THEN("Inventory should be still the same")
		{
			REQUIRE(main_thread_v2->getall() == "\"<Red>Ahh</>\", you cry while reaching for the door bell. Saying it was charched would be an understatement.\n");
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
	}
}
