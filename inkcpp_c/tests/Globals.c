#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <inkcpp.h>

#undef NDEBUG
#include <assert.h>

HInkStory*   story  = NULL;
HInkGlobals* store  = NULL;
HInkRunner*  thread = NULL;

void setup()
{
	if (! story) {
		story = ink_story_from_file(INK_TEST_RESOURCE_DIR "GlobalStory.bin");
	}
	if (store) {
		ink_globals_delete(store);
	}
	store = ink_story_new_globals(story);
	if (thread) {
		ink_runner_delete(thread);
	}
	thread = ink_story_new_runner(story, store);
}

int main()
{
	//====== Just reading Globals =====
	setup();

	// check story
	assert(
	    strcmp(
	        ink_runner_get_line(thread),
	        "My name is Jean Passepartout, but my friend's call me Jackie. I'm 23 years old.\n"
	    )
	    == 0
	);
	assert(strcmp(ink_runner_get_line(thread), "Foo:23\n") == 0);

	// check values in store
	InkValue val = ink_globals_get(store, "age");
	assert(val.type == ValueTypeInt32 && val.int32_v == 23);
	val = ink_globals_get(store, "friendly_name_of_player");
	assert(val.type == ValueTypeString && strcmp(val.string_v, "Jackie") == 0);


	//===== Modifing Globals =====
	setup();

	// set value of 'age'
	val.type    = ValueTypeInt32;
	val.int32_v = 30;
	assert(ink_globals_set(store, "age", val));

	// set value of 'friendl_name_of_player'
	val.type     = ValueTypeString;
	val.string_v = "Freddy";
	assert(ink_globals_set(store, "friendly_name_of_player", val));


	// check story output
	assert(
	    strcmp(
	        ink_runner_get_line(thread),
	        "My name is Jean Passepartout, but my friend's call me Freddy. I'm 30 years old.\n"
	    )
	    == 0
	);
	assert(strcmp(ink_runner_get_line(thread), "Foo:30\n") == 0);

	// check variable content
	val = ink_globals_get(store, "age");
	assert(val.type == ValueTypeInt32 && val.int32_v == 30);
	val = ink_globals_get(store, "friendly_name_of_player");
	assert(val.type == ValueTypeString && strcmp(val.string_v, "Freddy") == 0);
	val = ink_globals_get(store, "concat");
	assert(val.type == ValueTypeString && strcmp(val.string_v, "Foo:30") == 0);

	//===== Fail to set variables with invalid types or non existing variables =====
	setup();
	val = ink_globals_get(store, "foo");
	assert(val.type == ValueTypeNone);
	// not existing variable
	val.type     = ValueTypeString;
	val.string_v = "o";
	assert(! ink_globals_set(store, "foo", val));
	val.type     = ValueTypeString;
	val.string_v = "o";
	// wrong type
	assert(! ink_globals_set(store, "age", val));

	return 0;
}
