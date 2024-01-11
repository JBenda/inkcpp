#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <inkcpp.h>

#undef NDEBUG
#include <assert.h>

int main()
{
	HInkStory*   story  = ink_story_from_file(INK_TEST_RESOURCE_DIR "ListStory.bin");
	HInkGlobals* store  = ink_story_new_globals(story);
	HInkRunner*  runner = ink_story_new_runner(story, store);

	InkValue val = ink_globals_get(store, "list");
	assert(val.type == ValueTypeList);
	HInkList* list = val.list_v;

	InkListIter iter;

	// iterate through all flags
	{
		int hits[3] = {0};
		if (ink_list_flags(list, &iter)) {
			do {
				if (strcmp(iter.flag_name, "bird") == 0 && strcmp(iter.list_name, "animals") == 0) {
					hits[0] = 1;
				} else if (strcmp(iter.flag_name, "red") == 0 && strcmp(iter.list_name, "colors") == 0) {
					hits[1] = 1;
				} else if (strcmp(iter.flag_name, "yellow") == 0 && strcmp(iter.list_name, "colors") == 0) {
					hits[2] = 1;
				} else {
					assert(0);
				}
			} while (ink_list_iter_next(&iter));
			assert(hits[0] && hits[1] && hits[2]);
		}
	}
	// through all animals in list
	{
		int hits[2] = {0};
		if (ink_list_flags_from(list, "colors", &iter)) {
			do {
				if (strcmp(iter.flag_name, "red") == 0 && strcmp(iter.list_name, "colors") == 0) {
					hits[0] = 1;
				} else if (strcmp(iter.flag_name, "yellow") == 0 && strcmp(iter.list_name, "colors") == 0) {
					hits[1] = 1;
				} else {
					assert(0);
				}
			} while (ink_list_iter_next(&iter));
		}
		assert(hits[0] && hits[1]);
	}

	assert(ink_list_contains(list, "yellow"));
	assert(ink_list_contains(list, "white") == 0);

	ink_list_add(list, "white");
	ink_list_remove(list, "yellow");
	assert(ink_list_contains(list, "yellow") == 0);
	assert(ink_list_contains(list, "white"));

	assert(ink_globals_set(store, "list", val));

	assert(strcmp(ink_runner_get_line(runner), "cat, snake\n") == 0);
	assert(ink_runner_num_choices(runner) == 2);
	const HInkChoice* choice = ink_runner_get_choice(runner, 0);
	assert(strcmp(ink_choice_text(choice), "list: bird, white, red") == 0);
	return 0;
}
