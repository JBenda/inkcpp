VAR do_not_migrate = 10
VAR do_migrate = 15
->NewNode
=== Node1
A
-> Main
=== NewNode
B
-> Main
=== Main
{TURNS_SINCE(-> Node1)} {TURNS_SINCE(-> NewNode)} {TURNS_SINCE(-> Main)}
This is a simple story.
* A
* B
- catch
{TURNS_SINCE(-> Node1)} {TURNS_SINCE(-> NewNode)} {TURNS_SINCE(-> Main)}
Oh.
->DONE
