#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <ink/c/inkcpp.h> // if <os>-lib.zip was used for the installation

// #include <ink/inkcpp.h> // if <os>-clib.zip was used for the installation

InkValue ink_add(int argc, const InkValue argv[])
{
	assert(argc == 2);
	assert(argv[0].type == ValueTypeInt32 && argv[1].type == ValueTypeInt32);
	return (InkValue){.type = ValueTypeInt32, .int32_v = argv[0].int32_v + argv[1].int32_v};
}

int main()
{
	ink_compile_json("test.ink.json", "test.bin", NULL);
	HInkStory*  story  = ink_story_from_file("test.bin");
	HInkRunner* runner = ink_story_new_runner(story, NULL);

	ink_runner_bind(runner, "my_ink_function", ink_add, 1);

	while (1) {
		while (ink_runner_can_continue(runner)) {
			printf("%s", ink_runner_get_line(runner));
		}
		if (ink_runner_num_choices(runner) == 0)
			break;
		for (int i = 0; i < ink_runner_num_choices(runner); ++i) {
			printf("%i. %s\n", i, ink_choice_text(ink_runner_get_choice(runner, i)));
		}

		int id;
		scanf("%i", &id);
		ink_runner_choose(runner, id);
	}
}
