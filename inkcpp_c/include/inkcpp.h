#ifndef _INKCPP_H
#define _INKCPP_H

#include <stdint.h>


struct HInkSnapshot;
HInkSnapshot* ink_snapshot_from_file(const char* filename);
int ink_snapshot_num_runners(const HInkSnapshot* snapshot);
void ink_snapshot_write_to_file(const HInkSnapshot* snapshot, const char* filename);

struct HInkChoice;
const char* ink_choice_text(const HInkChoice* choice);
int ink_choice_num_tags(const HInkChoice* choice);
const char* ink_choice_get_tag(const HInkChoice* choice, int tag_id);

struct HInkList;
struct HInkListIter;
struct InkFlag {
  const char* flag_name;
  const char* list_name;
};
void ink_list_add(HInkList* list, const char* flag_name);
void ink_list_remove(HInkList* list, const char* flag_name);
bool ink_list_contains(const HInkList* list, const char* flag_name);
HInkListIter* ink_list_flags(const HInkList* list);
HInkListIter* ink_list_flags_from(const HInkList* list, const char* list_name);
InkFlag ink_list_iter_next(HInkListIter* iter);

struct InkValue {
  union {
    int bool_v;
    int32_t int32_v;
    uint32_t uint32_v;
    const char* string_v;
    HInkList* list_v;
  };
  enum Type {
    Bool,
    Uint32,
    Int32,
    String,
    Float,
    List
  } type;
};
const char* ink_value_to_string(const InkValue* value);

typedef void (*InkExternalFunctionVoid)(int argc, const InkValue* argv);
typedef void (*InkExternalFunction)(int argc, const InkValue* argv);

struct HInkRunner;
HInkSnapshot* ink_runner_create_snapshot(const HInkRunner* runner);
int ink_runner_can_continue(const HInkRunner* runner);
const char* ink_runner_get_line(HInkRunner* runner);
const char* ink_runner_get_all(HInkRunner* runner);
int ink_runner_num_tags(const HInkRunner* runner);
const char* ink_runner_tag(const HInkRunner* runner, int tag_id);
int ink_runner_num_choices(const HInkRunner* runner);

const HInkChoice* ink_runner_get_choice(const HInkRunner* runner, int choice_id);
void ink_runner_choose(HInkRunner* runner, int choice_id);
void ink_runner_bind_void(HInkRunner* runner, InkExternalFunctionVoid callback);
void ink_runner_bind(HInkRunner* runner, InkExternalFunction callback);

typedef void (*InkObserver)(InkValue new_value, const InkValue* old_value);

struct HInkGlobals;
HInkSnapshot* ink_globals_create_snapshot(const HInkGlobals* globals);
void ink_globals_observe(HInkGlobals* globals, InkObserver* observer);
InkValue ink_globals_get(const HInkGlobals* globals, const char* variable_name);
bool ink_globals_set(HInkGlobals* globals, const char* variable_name, InkValue value);

struct HInkStory;
HInkStory* ink_story_from_file(const char* filename);
HInkGlobals* ink_story_new_globals(HInkStory* story);
HInkRunner* ink_story_new_runner(HInkStory* story, HInkGlobals* globals);
HInkGlobals* ink_story_new_globals_from_snapshot(HInkStory* story, const HInkSnapshot* snapshot);
HInkRunner* ink_story_runner_from_snapshot(HInkStory* story, const HInkSnapshot* snapshot, HInkGlobals* globals);

void ink_compile_json(const char* input_filename, const char* output_filename, const char** error);

#endif
