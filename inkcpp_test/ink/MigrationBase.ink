# test:migration
# flavor:base

VAR do_not_migrate = 10
VAR do_migrate = 15
->Node1
=== OldNode
O
-> Node1
=== Node1
A
-> Main
=== Main
# knot:Main
~ temp tKeep = 2
~ temp tOld = 3
{TURNS_SINCE(-> Node1)} {TURNS_SINCE(-> OldNode)} {TURNS_SINCE(-> Main)}
This is a simple story.
~ tKeep = 5
* A
* B
- catch
{tKeep} {tOld}
{TURNS_SINCE(-> Node1)} {TURNS_SINCE(-> OldNode)} {TURNS_SINCE(-> Main)}
Oh.
->DONE
