#!/bin/python

import inkcpp_py
import sys
import os

story_ink = 'unreal_example.ink'
story_json = 'unreal_example.ink.json'
story_bin = 'unreal_example.bin'


# convert .ink / .json file to .bin file
if not os.path.exists(story_bin):
    if not os.path.exists(story_json):
        os.system('inklecate {}'.format(story_ink))
    inkcpp_py.compile_json(story_json, story_bin)

# load story and maybe snapshot
story = inkcpp_py.Story.from_file(story_bin)
if len(sys.argv) > 1:
    snap = inkcpp_py.Snapshot.from_file(sys.argv[1])
    globals = story.new_globals_from_snapshot(snap)
    runner = story.new_runner_from_snapshot(snap, globals)
else:
    globals = story.new_globals()
    runner = story.new_runner(globals)

# access global variables
print("Date: ", globals.date)
globals.date = inkcpp_py.Value("17.12.2023")
bg = globals.background.as_list();
print(bg)
bg.add('b')
print(bg)
bg.remove('a')
print(bg)
globals.background = inkcpp_py.Value(bg)

# observer examples
def ob_ping():
    print("chang^-^")
def ob_value(x):
    print("now: ", x)
def ob_delta(x, y):
    print("from:",y,"to:",x)

globals.observe_ping("brightness", ob_ping)
globals.observe_value("brightness", ob_value)
globals.observe_delta("brightness", ob_delta)


# external function with no input, but return value
def greeting(a):
    return inkcpp_py.Value("Tach")
runner.bind("GetGreeting", greeting)

# external function with no return value
def brightness(args):
    print("Set Brightness: ", args[0])
runner.bind_void("SetBrightness", brightness)

# simple story stepper
while True:
    while runner.can_continue():
        print(runner.getline())
    print("# tags: ", end="")
    print(', '.join(runner.tags()))
    if runner.has_choices():
        print()
        index = 1
        for c in runner:
            print(str(index) + ": " + c.text())
            print("\t"+', '.join(c.tags()))
            index += 1
        index = int(input('Select choice to continue: '))
        if index == -1:
            snap = runner.create_snapshot();
            snap.write_to_file("story.snap");
            break
        else:
            runner.choose(index - 1)
    else:
        break
print("Finish")
