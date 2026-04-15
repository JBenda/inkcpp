# source https://github.com/harryr0se from issue: #112
LIST activities = Swimming, SandCastle
VAR completed = ()

-> holiday
=== holiday
We're going to the seaside!
{completed: So far we've done the following: {completed}}
+ [Make a sand castle] -> sand_castle -> holiday
* [Go swimming] -> swimming -> holiday
* Time to go home -> home

= sand_castle
We made a great sand castle, it even has a moat!
~ completed += SandCastle
->->

= swimming
We swim and swam, it was delightful!
~ completed += Swimming
->->

= home
What a nice holiday that was
-> END
