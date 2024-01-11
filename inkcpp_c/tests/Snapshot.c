#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <inkcpp.h>

void check_end(HInkRunner* runner)
{
	assert(ink_runner_num_choices(runner) == 3);
	ink_runner_choose(runner, 2);
	while (ink_runner_can_continue(runner)) {
		ink_runner_get_line(runner);
	}
	assert(ink_runner_num_choices(runner) == 2);
}

int main()
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

	return 0;
}
