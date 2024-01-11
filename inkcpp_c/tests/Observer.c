#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <inkcpp.h>

#undef NDEBUG
#include <assert.h>

int cnt = 0;

void observer(InkValue new_value, InkValue old_value)
{
	if (cnt++ == 0) {
		assert(new_value.type == ValueTypeInt32 && new_value.int32_v == 1);
		assert(old_value.type == ValueTypeNone);
	} else {
		assert(new_value.type == ValueTypeInt32 && new_value.int32_v == 5);
		assert(old_value.type == ValueTypeInt32 && old_value.int32_v == 1);
	}
}

int main()
{
	HInkStory*   story  = ink_story_from_file(INK_TEST_RESOURCE_DIR "ObserverStory.bin");
	HInkGlobals* store  = ink_story_new_globals(story);
	HInkRunner*  thread = ink_story_new_runner(story, store);

	ink_globals_observe(store, "var1", observer);

	assert(strcmp(ink_runner_get_line(thread), "hello line 1 1 hello line 2 5 test line 3 5\n") == 0);
	assert(cnt == 2);

	return 0;
}
