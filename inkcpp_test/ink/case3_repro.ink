VAR x = ""
VAR cond = true
~ x = "prefix {cond:
line one
- else:
line two
} suffix"
{x}
-> END
