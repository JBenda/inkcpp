#include <string.h>
#include <stdbool.h>

#include <inkcpp.h>

#undef NDEBUG
#include <assert.h>

int main(void)
{
	HInkStory*   story = ink_story_from_file(INK_TEST_RESOURCE_DIR "WhitespaceMode.bin");
	HInkGlobals* store = ink_story_new_globals(story);

	// the default collapses a run to one space, as the reference ink runtime does
	HInkRunner* collapse = ink_story_new_runner(story, store);
	assert(strcmp(ink_runner_get_line(collapse), "A B\n") == 0);
	assert(ink_runner_num_choices(collapse) == 1);
	assert(strcmp(ink_choice_text(ink_runner_get_choice(collapse, 0)), "Pick me") == 0);

	// with INK_WHITESPACE_KEEP_RUNS the runs survive, in lines and in choice text
	HInkRunner* keep = ink_story_new_runner(story, store);
	ink_runner_set_whitespace_mode(keep, INK_WHITESPACE_KEEP_RUNS);
	assert(strcmp(ink_runner_get_line(keep), "A    B\n") == 0);
	assert(ink_runner_num_choices(keep) == 1);
	assert(strcmp(ink_choice_text(ink_runner_get_choice(keep, 0)), "Pick   me") == 0);

	// setting it back gives the default again
	HInkRunner* back = ink_story_new_runner(story, store);
	ink_runner_set_whitespace_mode(back, INK_WHITESPACE_KEEP_RUNS);
	ink_runner_set_whitespace_mode(back, INK_WHITESPACE_COLLAPSE);
	ink_runner_set_rng_seed(back, 1337);
	assert(strcmp(ink_runner_get_line(back), "A B\n") == 0);
}
