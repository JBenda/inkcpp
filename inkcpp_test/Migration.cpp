#include "catch.hpp"
#include "../snapshot_impl.h"

#include <memory>
#include <story.h>
#include <globals.h>
#include <choice.h>
#include <runner.h>
#include <compiler.h>
#include <snapshot.h>

using namespace ink::runtime;

SCENARIO("Simple isolated migration tests.", "[migration][integration]")
{
	GIVEN("a loaded base story with a fresh runner")
	{
		std::unique_ptr<story> base_story{story::from_file(INK_TEST_RESOURCE_DIR "MigrationBase.bin")};
		globals                base_globals = base_story->new_globals();
		runner                 base_thread  = base_story->new_runner(base_globals);

		WHEN("the base story is run")
		{
			std::string content = base_thread->getall();

			THEN("the intro is output and choices are available")
			{
				REQUIRE(base_thread->has_choices());
				REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
			}

			THEN("initial global variable values are set correctly")
			{
				CHECK(base_globals->get<int32_t>("do_not_migrate").value_or(0) == 10);
				CHECK(base_globals->get<int32_t>("do_migrate").value_or(0) == 15);
			}

			THEN("global and knot tags are correct")
			{
				REQUIRE(base_thread->num_global_tags() == 2);
				REQUIRE(base_thread->get_global_tag(0) == std::string("test:migration"));
				REQUIRE(base_thread->get_global_tag(1) == std::string("flavor:base"));
				REQUIRE(base_thread->num_knot_tags() == 1);
				REQUIRE(base_thread->get_knot_tag(0) == std::string("knot:Main"));
			}

			AND_WHEN("the first choice is taken")
			{
				REQUIRE(base_thread->has_choices()); // guard
				base_thread->choose(0);

				THEN("the story outputs the expected follow-up content")
				{
					REQUIRE(base_thread->getall() == "A\ncatch\n5 3\n1 -1 1\n1 0 1\nOh.\n");
				}
			}
		}

		GIVEN("and a variant story with changed global variables")
		{
			std::unique_ptr<story> new_story{story::from_file(INK_TEST_RESOURCE_DIR
			                                                  "MigrationChangeGlobals.bin")};

			WHEN("the new story is run fresh from the start")
			{
				globals     new_globals = new_story->new_globals();
				runner      new_thread  = new_story->new_runner(new_globals);
				std::string content     = new_thread->getall();

				THEN("the output matches the base story")
				{
					REQUIRE(new_thread->has_choices());
					REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
				}

				THEN("the updated global variable values are in effect")
				{
					CHECK(new_globals->get<int32_t>("do_migrate").value_or(0) == 10);
					CHECK(new_globals->get<int32_t>("new_var").value_or(0) == 20);
				}

				AND_WHEN("a choice is made")
				{
					REQUIRE(new_thread->has_choices()); // guard
					new_thread->choose(0);

					THEN("the story continues with updated variable output")
					{
						REQUIRE(new_thread->getall() == "A\ncatch\n1 -1 1\nOh.\n");
					}
				}
			}

			WHEN("the base story is run to a checkpoint and migrated to the new story")
			{
				std::string content = base_thread->getall();
				REQUIRE(base_thread->has_choices());
				REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
				base_thread->choose(0);
				std::unique_ptr<snapshot> snap{base_thread->create_snapshot()};
				REQUIRE(snap->can_be_migrated());
				globals new_globals = new_story->new_globals_from_snapshot(*snap);
				runner  new_thread  = new_story->new_runner_from_snapshot(*snap, new_globals);

				THEN("removed variable is gone, migrated value is kept, new variable defaults")
				{
					CHECK_FALSE(new_globals->get<int32_t>("do_not_migrate").has_value());
					CHECK(new_globals->get<int32_t>("do_migrate").value_or(0) == 15);
					CHECK(new_globals->get<int32_t>("new_var").value_or(0) == 20);
				}

				THEN("the story continues normally from the migration point")
				{
					REQUIRE(new_thread->getall() == "A\ncatch\n1 -1 1\nOh.\n");
				}
			}
		}

		GIVEN("and a variant story with changed knots")
		{
			std::unique_ptr<story> new_story{story::from_file(INK_TEST_RESOURCE_DIR
			                                                  "MigrationChangeNodes.bin")};

			WHEN("the new story is run fresh from the start")
			{
				globals     new_globals = new_story->new_globals();
				runner      new_thread  = new_story->new_runner(new_globals);
				std::string content     = new_thread->getall();

				THEN("the new story's intro is shown with modified visit counts")
				{
					REQUIRE(new_thread->has_choices());
					REQUIRE(content == "B\n-1 0 0\nThis is a simple story.\n");
				}

				AND_WHEN("a choice is made")
				{
					REQUIRE(new_thread->has_choices()); // guard
					new_thread->choose(0);

					THEN("the story continues with the new knot's output")
					{
						REQUIRE(new_thread->getall() == "A\ncatch\n-1 1 1\n0 1 1\nOh.\n");
					}
				}
			}

			WHEN("the base story is run to a checkpoint and migrated to the new story")
			{
				std::string content = base_thread->getall();
				REQUIRE(base_thread->has_choices());
				REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
				base_thread->choose(0);
				std::unique_ptr<snapshot> snap{base_thread->create_snapshot()};
				REQUIRE(snap->can_be_migrated());
				globals new_globals = new_story->new_globals_from_snapshot(*snap);
				runner  new_thread  = new_story->new_runner_from_snapshot(*snap, new_globals);

				THEN("visit counts are migrated and the story continues with node-aware output")
				{
					REQUIRE(new_thread->getall() == "A\ncatch\n1 -1 1\n1 0 1\nOh.\n");
				}
			}
		}

		GIVEN("and a variant story with changed temporary variables")
		{
			std::unique_ptr<story> new_story{story::from_file(INK_TEST_RESOURCE_DIR "MigrationTemp.bin")};

			WHEN("the new story is run fresh from the start")
			{
				globals     new_globals = new_story->new_globals();
				runner      new_thread  = new_story->new_runner(new_globals);
				std::string content     = new_thread->getall();

				THEN("the intro is shown at the expected knot")
				{
					REQUIRE(new_thread->has_choices());
					REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
					REQUIRE(new_thread->get_current_knot() == 0x25e83b84);
				}

				AND_WHEN("a choice is made")
				{
					REQUIRE(new_thread->has_choices()); // guard
					new_thread->choose(0);
					std::string after = new_thread->getall();

					THEN("the story continues with updated temporary variable output at the same knot")
					{
						REQUIRE(new_thread->get_current_knot() == 0x25e83b84);
						REQUIRE(after == "A\ncatch\n2 - 3\n1 -1 1\n1 0 1\nOh.\n");
					}
				}
			}

			WHEN("the base story is run to a checkpoint and migrated to the new story")
			{
				std::string content = base_thread->getall();
				REQUIRE(base_thread->has_choices());
				REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
				REQUIRE(base_thread->get_current_knot() == 0x25e83b84);
				base_thread->choose(0);
				REQUIRE(base_thread->get_current_knot() == 0x25e83b84);
				std::unique_ptr<snapshot> snap{base_thread->create_snapshot()};
				REQUIRE(snap->can_be_migrated());
				globals     new_globals = new_story->new_globals_from_snapshot(*snap);
				runner      new_thread  = new_story->new_runner_from_snapshot(*snap, new_globals);
				std::string after       = new_thread->getall();

				THEN("the old temporary variable is preserved and the new one uses its default")
				{
					REQUIRE(new_thread->get_current_knot() == 0x25e83b84);
					REQUIRE(after == "A\ncatch\n5 - 6\n1 -1 1\n1 0 1\nOh.\n");
				}
			}
		}

		GIVEN("and a variant story with changed knot and global tags")
		{
			std::unique_ptr<story> new_story{story::from_file(INK_TEST_RESOURCE_DIR
			                                                  "MigrationKnotTags.bin")};

			WHEN("the new story is run fresh from the start")
			{
				globals     new_globals = new_story->new_globals();
				runner      new_thread  = new_story->new_runner(new_globals);
				std::string content     = new_thread->getall();

				THEN("the new global and knot tags are present")
				{
					REQUIRE(new_thread->has_choices());
					REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
					REQUIRE(new_thread->num_global_tags() == 2);
					REQUIRE(new_thread->get_global_tag(0) == std::string("test:migration"));
					REQUIRE(new_thread->get_global_tag(1) == std::string("flavor:changed"));
					REQUIRE(new_thread->num_knot_tags() == 1);
					REQUIRE(new_thread->get_knot_tag(0) == std::string("knot:different"));
				}
			}

			WHEN("the base story is run to a checkpoint and migrated to the new story")
			{
				std::string content = base_thread->getall();
				REQUIRE(base_thread->has_choices());
				REQUIRE(content == "A\n0 -1 0\nThis is a simple story.\n");
				REQUIRE(base_thread->num_global_tags() == 2);
				REQUIRE(base_thread->get_global_tag(0) == std::string("test:migration"));
				REQUIRE(base_thread->get_global_tag(1) == std::string("flavor:base"));
				REQUIRE(base_thread->num_knot_tags() == 1);
				REQUIRE(base_thread->get_knot_tag(0) == std::string("knot:Main"));
				base_thread->choose(0);
				std::unique_ptr<snapshot> snap{base_thread->create_snapshot()};
				REQUIRE(snap->can_be_migrated());
				globals new_globals = new_story->new_globals_from_snapshot(*snap);
				runner  new_thread  = new_story->new_runner_from_snapshot(*snap, new_globals);

				THEN("the new story's global and knot tags are reflected after migration")
				{
					REQUIRE(new_thread->num_global_tags() == 2);
					REQUIRE(new_thread->get_global_tag(0) == std::string("test:migration"));
					REQUIRE(new_thread->get_global_tag(1) == std::string("flavor:changed"));
					REQUIRE(new_thread->num_knot_tags() == 1);
					REQUIRE(new_thread->get_knot_tag(0) == std::string("knot:different"));
				}

				THEN("the story continues normally from the migration point")
				{
					REQUIRE(new_thread->getall() == "A\ncatch\n5 3\n1 -1 1\n1 0 1\nOh.\n");
				}
			}
		}
	}
}

SCENARIO("Migration Test for small story", "[migration][integration]")
{
	GIVEN("a 'before' and 'after' version of the same story are loaded")
	{
		std::unique_ptr<story> before{story::from_file(INK_TEST_RESOURCE_DIR "MigrationBefore.bin")};
		std::unique_ptr<story> after{story::from_file(INK_TEST_RESOURCE_DIR "MigrationAfter.bin")};

		WHEN("the 'before' story is played through sand castle then swimming and a snapshot is taken")
		{
			runner thread_before = before->new_runner();

			// Advance through initial state
			REQUIRE(thread_before->getall() == "We're going to the seaside!\n");
			REQUIRE(thread_before->num_choices() == 3);
			CHECK(thread_before->get_choice(0)->text() == std::string("Make a sand castle"));
			CHECK(thread_before->get_choice(1)->text() == std::string("Go swimming"));
			CHECK(thread_before->get_choice(2)->text() == std::string("Time to go home"));

			thread_before->choose(0); // sand castle
			REQUIRE(
			    thread_before->getall()
			    == "We made a great sand castle, it even has a moat!\nWe're going to the seaside!\nSo far "
			       "we've done the following: SandCastle\n"
			);
			REQUIRE(thread_before->num_choices() == 3);
			CHECK(thread_before->get_choice(0)->text() == std::string("Make a sand castle"));
			CHECK(thread_before->get_choice(1)->text() == std::string("Go swimming"));
			CHECK(thread_before->get_choice(2)->text() == std::string("Time to go home"));

			thread_before->choose(1); // swimming — snapshot point
			std::unique_ptr<snapshot> snap{thread_before->create_snapshot()};
			REQUIRE(snap->can_be_migrated());

			THEN("the 'before' story continues from the swimming point with only the remaining choices")
			{
				REQUIRE(
				    thread_before->getall()
				    == "We swim and swam, it was delightful!\nWe're going to the seaside!\nSo far we've "
				       "done the following: Swimming, SandCastle\n"
				);
				REQUIRE(thread_before->num_choices() == 2);
				CHECK(thread_before->get_choice(0)->text() == std::string("Make a sand castle"));
				CHECK(thread_before->get_choice(1)->text() == std::string("Time to go home"));
			}

			AND_WHEN("the snapshot is loaded into the 'after' story")
			{
				runner      thread_after = after->new_runner_from_snapshot(*snap);
				std::string after_out    = thread_after->getall();

				THEN("the 'after' story resumes from the same point with the new Ice Cream choice")
				{
					REQUIRE(
					    after_out
					    == "We swim and swam, it was delightful!\nWe're going to the seaside!\nSo far "
					       "we've done the following: Swimming, SandCastle\n"
					);
					REQUIRE(thread_after->num_choices() == 3);
					CHECK(thread_after->get_choice(0)->text() == std::string("Make a sand castle"));
					CHECK(thread_after->get_choice(1)->text() == std::string("Get Ice Cream"));
					CHECK(thread_after->get_choice(2)->text() == std::string("Time to go home"));
				}

				AND_WHEN("the Ice Cream choice is selected")
				{
					REQUIRE(thread_after->num_choices() == 3); // guard
					thread_after->choose(1);

					THEN("the ice cream activity is shown with updated cumulative history")
					{
						REQUIRE(
						    thread_after->getall()
						    == "We got ice cream, mine was raspberry!\nWe're going to the seaside!\nSo far "
						       "we've done the following: Swimming, SandCastle, IceCream\n"
						);
					}
				}
			}
		}
	}
}

SCENARIO("Migratability of finished threads", "[migration][integration]")
{
	GIVEN("A story with multipl threads")
	{
		std::unique_ptr<story> base_story{story::from_file(INK_TEST_RESOURCE_DIR "UE_example.bin")};
		runner                 base_thread = base_story->new_runner();

		WHEN("running a empty thread")
		{
			base_thread->move_to("Wait");
			base_thread->getall();
			THEN("the story should be migratable")
			{
				std::unique_ptr<snapshot> snap{base_thread->create_snapshot()};
				REQUIRE(snap->can_be_migrated());
			}
		}

		WHEN("running an non empty thread to the end")
		{
			base_thread->move_to("TPotions.TTalkWithAnimals");
			REQUIRE(base_thread->getall() == "A potion which allows the consumer to talk with a variaty of animals. Just make sure\nyour serroundings do not think you are crazy.\n");
			REQUIRE(base_thread->num_choices() == 2);
			base_thread->choose(1);
			REQUIRE(
			    base_thread->getall() == "A take a sip. The potion tastes like Hores, it is afull.\n"
			);

			THEN("the story should be migratable")
			{
				std::unique_ptr<snapshot> snap{base_thread->create_snapshot()};
				REQUIRE(snap->can_be_migrated());
			}
		}
	}
}

SCENARIO("Migratibility of the UE_example story", "[migration][intigration]")
{
	GIVEN("The UE examplestory v1 and v2")
	{
		std::unique_ptr<story> story_v1{story::from_file(INK_TEST_RESOURCE_DIR "UE_example.bin")};
		std::unique_ptr<story> story_v2{story::from_file(INK_TEST_RESOURCE_DIR "UE_example_v2.bin")};
		runner                 thread = story_v1->new_runner();
		std::string            last_target{};
		auto                   callback = [&last_target](const char* target) {
      last_target = target;
		};
		thread->bind("transition", callback, false);
		WHEN("go to mansion in v1")
		{
			REQUIRE(
			    thread->getall() == "You step outside your car. Its a wired feeling beehing here again.\n"
			);
			REQUIRE(last_target == "");
			REQUIRE(thread->num_choices() == 2);
			thread->choose(1);
			THEN("expect normal output")
			{
				REQUIRE(thread->getall() == "You startk walking to Mansion.Entrance.\nJust in time you are able to see the door, someone with with a yellow summer dress enters it.\nYou're climbing the 56 steps up to the door; high tides are an annoying thing.\n");
				REQUIRE(last_target == "Mansion.Entrance");
			}
			AND_WHEN("knock in v1")
			{
				REQUIRE(thread->getall() == "You startk walking to Mansion.Entrance.\nJust in time you are able to see the door, someone with with a yellow summer dress enters it.\nYou're climbing the 56 steps up to the door; high tides are an annoying thing.\n");
				REQUIRE(thread->num_choices() == 2);
				thread->choose(1);
				THEN("expect normal output")
				{
					REQUIRE(thread->getall() == "\"<Red>Ahh</>\", you cry while reaching for the door bell. Saying it was charched would be an understatement.\n");
				}
			}
		}
		WHEN("go to mansion in v1, but get after choice text in v2")
		{
			REQUIRE(
			    thread->getall() == "You step outside your car. Its a wired feeling beehing here again.\n"
			);
			REQUIRE(last_target == "");
			REQUIRE(thread->num_choices() == 2);
			thread->choose(1);
			std::unique_ptr<snapshot> snap{thread->create_snapshot()};
			REQUIRE(snap->can_be_migrated());
			runner thread2 = story_v2->new_runner_from_snapshot(*snap);
			thread2->bind("transition", callback, false);
			THEN("the new text at the same location should be displayed")
			{
				REQUIRE(thread2->getall() == "You startk walking to Mansion.Entrance.\nYou're climbing the 56 steps up to the door; high tides are an annoying thing.\n");
				REQUIRE(last_target == "Mansion.Entrance");
			}
		}
		WHEN("knock in v1, but get after choice text in v2")
		{
			thread->getall();
			thread->choose(1);
			thread->getall();
			thread->choose(1);
			std::unique_ptr<snapshot> snap{thread->create_snapshot()};
			REQUIRE(snap->can_be_migrated());
			runner thread2 = story_v2->new_runner_from_snapshot(*snap);
			thread2->bind("transition", callback, false);
			THEN("the new text at the same location should be displayed")
			{
				REQUIRE(thread2->getall() == "\"<Red>Ahh</>\", you cry while reaching for the door bell. Saying it was charched would be an understatement.\n");
			}
		}
	}
}
