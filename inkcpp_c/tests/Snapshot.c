#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <inkcpp.h>

#undef NDEBUG
#include <assert.h>

void check_end(HInkRunner* runner)
{
	assert(ink_runner_num_choices(runner) == 3);
	ink_runner_choose(runner, 2);
	while (ink_runner_can_continue(runner)) {
		ink_runner_get_line(runner);
	}
	assert(ink_runner_num_choices(runner) == 2);
}

#define CHECK_CHOICE(RUNNER, IDX, STR) \
	assert(strcmp(ink_choice_text(ink_runner_get_choice(RUNNER, IDX)), STR) == 0)
#define CHECK_NEXT_LINE(RUNNER, STR) assert(strcmp(ink_runner_get_line(RUNNER), STR) == 0)

int main()
{
	{
		HInkStory*  story  = ink_story_from_file(INK_TEST_RESOURCE_DIR "SimpleStoryFlow.bin");
		HInkRunner* runner = ink_story_new_runner(story, NULL);

		ink_runner_get_line(runner);
		assert(ink_runner_num_choices(runner) == 3);
		ink_runner_choose(runner, 2);

		// snapshot after choose -> snapshot will print text after loading
		HInkSnapshot* snap1 = ink_runner_create_snapshot(runner);

		int cnt = 0;
		while (ink_runner_can_continue(runner)) {
			ink_runner_get_line(runner);
			++cnt;
		}

		// snapshot befroe choose, context (last output lines) can not bet optained at loading
		HInkSnapshot* snap2 = ink_runner_create_snapshot(runner);

		check_end(runner);


		ink_runner_delete(runner);
		runner = ink_story_new_runner_from_snapshot(story, snap1, NULL, 0);

		// same amount at output then before
		while (ink_runner_can_continue(runner)) {
			ink_runner_get_line(runner);
			--cnt;
		}
		assert(cnt == 0);

		check_end(runner);


		ink_runner_delete(runner);
		runner = ink_story_new_runner_from_snapshot(story, snap2, NULL, 0);

		assert(ink_runner_can_continue(runner) == 0);
		check_end(runner);
	}
	{
		HInkStory*  before_story  = ink_story_from_file(INK_TEST_RESOURCE_DIR "MigrationBefore.bin");
		HInkStory*  after_story   = ink_story_from_file(INK_TEST_RESOURCE_DIR "MigrationAfter.bin");
		HInkRunner* before_runner = ink_story_new_runner(before_story, NULL);
		CHECK_NEXT_LINE(before_runner, "We're going to the seaside!\n");
		assert(ink_runner_num_choices(before_runner) == 3);
		CHECK_CHOICE(before_runner, 0, "Make a sand castle");
		CHECK_CHOICE(before_runner, 1, "Go swimming");
		CHECK_CHOICE(before_runner, 2, "Time to go home");
		ink_runner_choose(before_runner, 0);

		CHECK_NEXT_LINE(before_runner, "We made a great sand castle, it even has a moat!\n");
		CHECK_NEXT_LINE(before_runner, "We're going to the seaside!\n");
		CHECK_NEXT_LINE(before_runner, "So far we've done the following: SandCastle\n");
		assert(ink_runner_num_choices(before_runner) == 3);
		CHECK_CHOICE(before_runner, 0, "Make a sand castle");
		CHECK_CHOICE(before_runner, 1, "Go swimming");
		CHECK_CHOICE(before_runner, 2, "Time to go home");
		ink_runner_choose(before_runner, 1);

		HInkSnapshot* snap = ink_runner_create_snapshot(before_runner);
		assert(ink_snapshot_can_be_migrated(snap));

		CHECK_NEXT_LINE(before_runner, "We swim and swam, it was delightful!\n");
		CHECK_NEXT_LINE(before_runner, "We're going to the seaside!\n");
		CHECK_NEXT_LINE(before_runner, "So far we've done the following: Swimming, SandCastle\n");
		assert(ink_runner_num_choices(before_runner) == 2);
		CHECK_CHOICE(before_runner, 0, "Make a sand castle");
		CHECK_CHOICE(before_runner, 1, "Time to go home");

		HInkRunner* after_runner = ink_story_new_runner_from_snapshot(after_story, snap, NULL, 0);

		CHECK_NEXT_LINE(after_runner, "We swim and swam, it was delightful!\n");
		CHECK_NEXT_LINE(after_runner, "We're going to the seaside!\n");
		CHECK_NEXT_LINE(after_runner, "So far we've done the following: Swimming, SandCastle\n");
		assert(ink_runner_num_choices(after_runner) == 3);
		CHECK_CHOICE(after_runner, 0, "Make a sand castle");
		CHECK_CHOICE(after_runner, 1, "Get Ice Cream");
		CHECK_CHOICE(after_runner, 2, "Time to go home");
		ink_runner_choose(after_runner, 1);

		CHECK_NEXT_LINE(after_runner, "We got ice cream, mine was raspberry!\n");
		CHECK_NEXT_LINE(after_runner, "We're going to the seaside!\n");
		CHECK_NEXT_LINE(
		    after_runner, "So far we've done the following: Swimming, SandCastle, IceCream\n"
		);
	}


	return 0;
}
