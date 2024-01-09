#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <inkcpp.h>

int cnt_my_sqrt = 0;

InkValue my_sqrt(int argc, const InkValue argv[])
{
	cnt_my_sqrt += 1;
	assert(argc == 1);
	InkValue v = argv[0];
	switch (v.type) {
		case ValueTypeFloat: v.float_v = sqrtf(v.float_v); break;
		case ValueTypeInt32: v.int32_v = sqrt(v.int32_v); break;
		case ValueTypeUint32: v.uint32_v = sqrtf(v.uint32_v); break;
		default: assert(0);
	}
	return v;
}

int cnt_greeting = 0;

InkValue greeting(int argc, const InkValue argv[])
{
	cnt_greeting += 1;
	assert(argc == 0);
	InkValue v;
	v.type     = ValueTypeString;
	v.string_v = "Hohooh";
	return v;
}

int main(int argc, const char* argv[])
{
	HInkStory*  story  = ink_story_from_file(INK_TEST_RESOURCE_DIR "FallBack.bin");
	HInkRunner* runner = ink_story_new_runner(story, NULL);
	ink_runner_bind(runner, "greeting", greeting);
	ink_runner_bind(runner,"sqrt",  my_sqrt);

	const char* res = ink_runner_get_line(runner);
	assert(strcmp(res, "Hohooh ! A small demonstration of my power:\n")== 0);
	assert(ink_runner_can_continue(runner));

	assert(strcmp(ink_runner_get_line(runner), "Math 4 * 4 = 16, stunning i would say\n")== 0);
	assert(ink_runner_can_continue(runner) == 0);

	assert(cnt_my_sqrt == 2);
	assert(cnt_greeting == 1);
	return 0;
}
