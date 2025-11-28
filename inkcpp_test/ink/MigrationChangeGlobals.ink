VAR do_migrate = 10
VAR new_var = 20
->Node1
=== OldNode
O
-> Node1
=== Node1
A
-> Main
=== Main
{TURNS_SINCE(-> Node1)} {TURNS_SINCE(-> OldNode)} {TURNS_SINCE(-> Main)}
This is a simple story.
* A
* B
- catch
{TURNS_SINCE(-> Node1)} {TURNS_SINCE(-> OldNode)} {TURNS_SINCE(-> Main)}
Oh.
->DONE
