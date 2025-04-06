#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <inkcpp.h>

#undef NDEBUG
#include <assert.h>

int main()
{
	HInkStory*   story  = ink_story_from_file(INK_TEST_RESOURCE_DIR "TagsStory.bin");
	HInkGlobals* store  = ink_story_new_globals(story);
	HInkRunner*  runner = ink_story_new_runner(story, store);

	assert(ink_runner_num_tags(runner) == 0);
	assert(ink_runner_num_knot_tags(runner) == 0);
	assert(ink_runner_num_global_tags(runner) == 0);
	assert(ink_runner_current_knot(runner) == 0);

	assert(strcmp(ink_runner_get_line(runner), "First line has global tags only\n") == 0);
	assert(ink_runner_current_knot(runner) == ink_hash_string("global_tags_only"));
	assert(ink_runner_num_tags(runner) == 1);
	assert(strcmp(ink_runner_tag(runner, 0), "global_tag") == 0);
	assert(ink_runner_num_global_tags(runner) == 1);
	assert(strcmp(ink_runner_global_tag(runner, 0), "global_tag") == 0);
	assert(ink_runner_num_knot_tags(runner) == 0);

	assert(strcmp(ink_runner_get_line(runner), "Second line has one tag\n") == 0);
	assert(ink_runner_current_knot(runner) == ink_hash_string("global_tags_only"));
	assert(ink_runner_num_tags(runner) == 1);
	assert(strcmp(ink_runner_tag(runner, 0), "tagged") == 0);
	assert(ink_runner_num_global_tags(runner) == 1);
	assert(strcmp(ink_runner_global_tag(runner, 0), "global_tag") == 0);
	assert(ink_runner_num_knot_tags(runner) == 0);

	assert(strcmp(ink_runner_get_line(runner), "Third line has two tags\n") == 0);
	assert(ink_runner_current_knot(runner) == ink_hash_string("global_tags_only"));
	assert(ink_runner_num_tags(runner) == 2);
	assert(strcmp(ink_runner_tag(runner, 0), "tag next line") == 0);
	assert(strcmp(ink_runner_tag(runner, 1), "more tags") == 0);
	assert(ink_runner_num_global_tags(runner) == 1);
	assert(strcmp(ink_runner_global_tag(runner, 0), "global_tag") == 0);
	assert(ink_runner_num_knot_tags(runner) == 0);

	assert(strcmp(ink_runner_get_line(runner), "Fourth line has three tags\n") == 0);
	assert(ink_runner_current_knot(runner) == ink_hash_string("global_tags_only"));
	assert(ink_runner_num_tags(runner) == 3);
	assert(strcmp(ink_runner_tag(runner, 0), "above") == 0);
	assert(strcmp(ink_runner_tag(runner, 1), "side") == 0);
	assert(strcmp(ink_runner_tag(runner, 2), "across") == 0);
	assert(ink_runner_num_global_tags(runner) == 1);
	assert(strcmp(ink_runner_global_tag(runner, 0), "global_tag") == 0);
	assert(ink_runner_num_knot_tags(runner) == 0);

	assert(strcmp(ink_runner_get_line(runner), "Hello\n") == 0);
	assert(ink_runner_current_knot(runner) == ink_hash_string("start"));
	assert(ink_runner_num_tags(runner) == 4);
	assert(strcmp(ink_runner_tag(runner, 0), "knot_tag_start") == 0);
	assert(strcmp(ink_runner_tag(runner, 1), "second_knot_tag_start") == 0);
	assert(strcmp(ink_runner_tag(runner, 2), "third_knot_tag") == 0);
	assert(strcmp(ink_runner_tag(runner, 3), "output_tag_h") == 0);
	assert(ink_runner_num_knot_tags(runner) == 3);
	assert(strcmp(ink_runner_knot_tag(runner, 0), "knot_tag_start") == 0);
	assert(strcmp(ink_runner_knot_tag(runner, 1), "second_knot_tag_start") == 0);
	assert(strcmp(ink_runner_knot_tag(runner, 2), "third_knot_tag") == 0);
	assert(ink_runner_num_global_tags(runner) == 1);
	assert(strcmp(ink_runner_global_tag(runner, 0), "global_tag") == 0);

	assert(strcmp(ink_runner_get_line(runner), "Second line has no tags\n") == 0);
	assert(ink_runner_num_tags(runner) == 0);
	assert(ink_runner_num_knot_tags(runner) == 3);
	assert(strcmp(ink_runner_knot_tag(runner, 0), "knot_tag_start") == 0);
	assert(strcmp(ink_runner_knot_tag(runner, 1), "second_knot_tag_start") == 0);
	assert(strcmp(ink_runner_knot_tag(runner, 2), "third_knot_tag") == 0);
	assert(ink_runner_num_global_tags(runner) == 1);
	assert(strcmp(ink_runner_global_tag(runner, 0), "global_tag") == 0);

	assert(! ink_runner_can_continue(runner));
	assert(ink_runner_num_choices(runner) == 2);
	const HInkChoice* choice;
	choice = ink_runner_get_choice(runner, 0);
	assert(ink_choice_num_tags(choice) == 0);
	assert(strcmp(ink_choice_text(choice), "a") == 0);
	choice = ink_runner_get_choice(runner, 1);
	assert(ink_choice_num_tags(choice) == 2);
	assert(strcmp(ink_choice_text(choice), "b") == 0);
	assert(strcmp(ink_choice_get_tag(choice, 0), "choice_tag_b") == 0);
	assert(strcmp(ink_choice_get_tag(choice, 1), "choice_tag_b_2") == 0);

	ink_runner_choose(runner, 1);
	assert(ink_runner_can_continue(runner));
	assert(strcmp(ink_runner_get_line(runner), "Knot2\n") == 0);
	assert(ink_runner_current_knot(runner) == ink_hash_string("knot2.sub"));
	assert(! ink_runner_can_continue(runner));
	assert(ink_runner_num_tags(runner) == 2);
	assert(strcmp(ink_runner_tag(runner, 0), "knot_tag_2") == 0);
	assert(strcmp(ink_runner_tag(runner, 1), "output_tag_k") == 0);
	assert(ink_runner_num_knot_tags(runner) == 1);
	assert(strcmp(ink_runner_knot_tag(runner, 0), "knot_tag_2") == 0);
	assert(ink_runner_num_global_tags(runner) == 1);
	assert(strcmp(ink_runner_global_tag(runner, 0), "global_tag") == 0);
	assert(ink_runner_num_choices(runner) == 3);
	choice = ink_runner_get_choice(runner, 0);
	assert(ink_choice_num_tags(choice) == 0);
	assert(strcmp(ink_choice_text(choice), "e") == 0);
	choice = ink_runner_get_choice(runner, 1);
	assert(strcmp(ink_choice_text(choice), "f with detail") == 0);
	assert(ink_choice_num_tags(choice) == 4);
	assert(strcmp(ink_choice_get_tag(choice, 0), "shared_tag") == 0);
	assert(strcmp(ink_choice_get_tag(choice, 1), "shared_tag_2") == 0);
	assert(strcmp(ink_choice_get_tag(choice, 2), "choice_tag") == 0);
	assert(strcmp(ink_choice_get_tag(choice, 3), "choice_tag_2") == 0);
	choice = ink_runner_get_choice(runner, 2);
	assert(strcmp(ink_choice_text(choice), "g") == 0);
	assert(ink_choice_num_tags(choice) == 1);
	assert(strcmp(ink_choice_get_tag(choice, 0), "choice_tag_g") == 0);
	assert(! ink_runner_can_continue(runner));

	ink_runner_choose(runner, 1);
	assert(ink_runner_can_continue(runner));
	assert(strcmp(ink_runner_get_line(runner), "f and content\n") == 0);
	assert(ink_runner_num_global_tags(runner) == 1);
	assert(strcmp(ink_runner_global_tag(runner, 0), "global_tag") == 0);
	assert(ink_runner_num_knot_tags(runner) == 1);
	assert(strcmp(ink_runner_knot_tag(runner, 0), "knot_tag_2") == 0);
	assert(ink_runner_num_tags(runner) == 4);
	assert(strcmp(ink_runner_tag(runner, 0), "shared_tag") == 0);
	assert(strcmp(ink_runner_tag(runner, 1), "shared_tag_2") == 0);
	assert(strcmp(ink_runner_tag(runner, 2), "content_tag") == 0);
	assert(strcmp(ink_runner_tag(runner, 3), "content_tag_2") == 0);


	assert(strcmp(ink_runner_get_line(runner), "out\n") == 0);
	assert(ink_runner_current_knot(runner) == ink_hash_string("knot2.sub"));
	assert(ink_runner_num_global_tags(runner) == 1);
	assert(strcmp(ink_runner_global_tag(runner, 0), "global_tag") == 0);
	assert(ink_runner_num_knot_tags(runner) == 1);
	assert(strcmp(ink_runner_knot_tag(runner, 0), "knot_tag_2") == 0);
	assert(ink_runner_num_tags(runner) == 1);

	assert(! ink_runner_can_continue(runner));
}
