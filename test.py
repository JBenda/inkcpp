import inkcpp_py as py

story = py.Story.from_file("UE_example.ink.bin")
glob = story.new_globals()
run = story.new_runner(glob)

def ob(new, old):
    l = new.as_list()
    print("P:")
    for f in l.flags_from("Potions"):
        print("\t", f.list_name, f.name)
    print("C:")
    for f in l.flags_from("Clues"):
        print("\t", f.list_name, f.name)

def ob_h(new, old):
    assert old is None
    print("type: ", new.type)
    # if old:
    #     print("O: ", old.type)

glob.observe("Inventory", ob)
glob.observe("Health", ob_h)

print(run.getall())

# run2 = story.new_runner(glob)

# print("\n\nRUN2\n")
# run2.move_to("TClues.TSkull")
# while run2.can_continue():
#     print(run2.getline())


run.choose(1)
print(run.getall())
print("#C", run.num_choices())
print(run.get_choice(1).text())
run.choose(1)
while run.can_continue():
    print("a")
    print(run.getline())

