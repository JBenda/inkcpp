VAR x = ""
VAR i = 0
-> loop

=== loop ===
{ i >= 4: -> done }
~ i = i + 1
~ x = "prefix {cycle:AAAA|BBBB|CCCC} suffix"
{x}
-> loop

=== done ===
After line.
-> END
